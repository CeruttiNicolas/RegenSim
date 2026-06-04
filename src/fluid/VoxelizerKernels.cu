#include "fluid/VoxelizerKernels.cuh"
#include <cuda_runtime.h>
#include <iostream>

#define CUDA_CHECK(call) \
do { \
    cudaError_t err = call; \
    if (err != cudaSuccess) { \
        std::cerr << "CUDA Error [" << cudaGetErrorName(err) << "]: " \
                  << cudaGetErrorString(err) \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(EXIT_FAILURE); \
    } \
} while(0)

__device__ bool rayTriangleIntersectDevice(const glm::dvec3& orig, const glm::dvec3& dir, const glm::dvec3& v0, const glm::dvec3& v1, const glm::dvec3& v2, double& t) {
    const double eps = 1e-7;
    glm::dvec3 edge1 = v1 - v0;
    glm::dvec3 edge2 = v2 - v0;
    glm::dvec3 h = glm::cross(dir, edge2);
    double a = glm::dot(edge1, h);

    if (a > -eps && a < eps) return false;

    double f = 1.0 / a;
    glm::dvec3 s = orig - v0;
    double u = f * glm::dot(s, h);
    if (u < -eps || u - 1.0 > eps) return false;

    glm::dvec3 q = glm::cross(s, edge1);
    double v = f * glm::dot(dir, q);
    if (v < -eps || u + v - 1.0 > eps) return false;

    t = f * glm::dot(edge2, q);
    return t > eps;
}

__global__ void scanlineVoxelizeKernel(
    const glm::dvec3* vertices, const uint32_t* indices, const TriangleAABB* aabbs, int numTriangles,
    uint8_t* voxelGrid,
    glm::dvec3 minBounds, glm::dvec3 maxBounds, double dx,
    int Nx, int Ny, int Nz
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int z = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= Nx || z >= Nz) return;

    double px = minBounds.x + (x + 0.5) * dx + (dx * 1e-4);
    double pz = minBounds.z + (z + 0.5) * dx + (dx * 1e-4);
    glm::dvec3 rayOrig(px, minBounds.y - (dx * 2.0), pz);
    glm::dvec3 rayDir(0.0, 1.0, 0.0);

    double hits[4];
    int hitCount = 0;

    for (int i = 0; i < numTriangles; i++) {
        if (px < aabbs[i].minX || px > aabbs[i].maxX ||
            pz < aabbs[i].minZ || pz > aabbs[i].maxZ) continue;

        glm::dvec3 v0 = vertices[indices[i * 3]];
        glm::dvec3 v1 = vertices[indices[i * 3 + 1]];
        glm::dvec3 v2 = vertices[indices[i * 3 + 2]];

        double t;
        if (rayTriangleIntersectDevice(rayOrig, rayDir, v0, v1, v2, t)) {
            if (hitCount < 4) {
                hits[hitCount] = rayOrig.y + t;
                hitCount++;
            }
        }
    }

    // Bubble Sort
    for (int i = 0; i < hitCount - 1; i++) {
        for (int j = 0; j < hitCount - i - 1; j++) {
            if (hits[j] > hits[j + 1]) {
                double temp = hits[j];
                hits[j] = hits[j + 1];
                hits[j + 1] = temp;
            }
        }
    }

    for (int y = 0; y < Ny; y++) {
        double currentY = minBounds.y + (y + 0.5) * dx;
        bool isInside = false;

        for (int h = 0; h < hitCount - 1; h += 2) {
            if (currentY > hits[h] && currentY < hits[h + 1]) {
                isInside = true;
                break;
            }
        }

        if (isInside) {
            size_t flatIdx = (size_t)x + ((size_t)y * Nx) + ((size_t)z * Nx * Ny);
            voxelGrid[flatIdx] = 1;
        }
    }
}

void launchVoxelizerKernel(
    const std::vector<glm::dvec3>& vertices,
    const std::vector<uint32_t>& indices,
    const std::vector<TriangleAABB>& aabbs,
    std::vector<uint8_t>& voxelGrid,
    glm::dvec3 minBounds, glm::dvec3 maxBounds, double dx,
    int Nx, int Ny, int Nz
) {
    size_t numVoxels = (size_t)Nx * Ny * Nz;
    int numTriangles = indices.size() / 3;

    glm::dvec3* d_vertices;
    uint32_t* d_indices;
    TriangleAABB* d_aabbs;
    uint8_t* d_voxelGrid;

    CUDA_CHECK(cudaMalloc(&d_vertices, vertices.size() * sizeof(glm::dvec3)));
    CUDA_CHECK(cudaMalloc(&d_indices, indices.size() * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(&d_aabbs, aabbs.size() * sizeof(TriangleAABB)));
    CUDA_CHECK(cudaMalloc(&d_voxelGrid, numVoxels * sizeof(uint8_t)));

    CUDA_CHECK(cudaMemset(d_voxelGrid, 0, numVoxels * sizeof(uint8_t)));

    CUDA_CHECK(cudaMemcpy(d_vertices, vertices.data(), vertices.size() * sizeof(glm::dvec3), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_indices, indices.data(), indices.size() * sizeof(uint32_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_aabbs, aabbs.data(), aabbs.size() * sizeof(TriangleAABB), cudaMemcpyHostToDevice));
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((Nx + threadsPerBlock.x - 1) / threadsPerBlock.x,
        (Nz + threadsPerBlock.y - 1) / threadsPerBlock.y);

    std::cout << "Launching GPU Kernel (2D Grid: " << numBlocks.x << "x" << numBlocks.y << " blocks)..." << std::endl;

    scanlineVoxelizeKernel <<<numBlocks, threadsPerBlock >>> (
        d_vertices, d_indices, d_aabbs, numTriangles,
        d_voxelGrid, minBounds, maxBounds, dx, Nx, Ny, Nz
        );

    CUDA_CHECK(cudaPeekAtLastError());

    CUDA_CHECK(cudaDeviceSynchronize());

	// Copy the results back to the host
    CUDA_CHECK(cudaMemcpy(voxelGrid.data(), d_voxelGrid, numVoxels * sizeof(uint8_t), cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_vertices));
    CUDA_CHECK(cudaFree(d_indices));
    CUDA_CHECK(cudaFree(d_aabbs));
    CUDA_CHECK(cudaFree(d_voxelGrid));
}