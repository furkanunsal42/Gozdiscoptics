#include "gtc/constants.hpp"
#include <string>
constexpr double M_PI = glm::pi<double>();

#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// ------------------ Constants ------------------
constexpr double c0 = 299792458.0;
constexpr double eps0 = 8.854187817e-12;
constexpr double mu0 = 4.0 * M_PI * 1e-7;

// ------------------ PNG writer ------------------
void save_png(const std::vector<std::vector<double>>& data,
    int Nx, int Ny, const std::string& name)
{
    std::vector<unsigned char> img(Nx * Ny);

    double maxv = 0.0;
    for (int i = 0; i < Nx; ++i)
        for (int j = 0; j < Ny; ++j)
            maxv = std::max(maxv, data[i][j]);

    if (maxv == 0.0) maxv = 1.0;

    for (int j = 0; j < Ny; ++j)
        for (int i = 0; i < Nx; ++i) {
            double v = data[i][j] / maxv;
            img[j * Nx + i] =
                (unsigned char)(255.0 * std::clamp(v, 0.0, 1.0));
        }

    stbi_write_png(name.c_str(), Nx, Ny, 1, img.data(), Nx);
}

// ------------------ Main ------------------
int main()
{
    // -------- Grid & physics (CRITICAL choices) --------
    const int Nx = 2400;
    const int Ny = 800;

    const double dx = 2e-3;
    const double dy = dx;
    const double dt = dx / (2.2 * c0);

    const int Nt = 5000;

    const double f0 = 2e9;
    const double omega = 2.0 * M_PI * f0;

    // -------- Fields --------
    std::vector<std::vector<double>> Ez(Nx, std::vector<double>(Ny, 0.0));
    std::vector<std::vector<double>> Hx(Nx, std::vector<double>(Ny, 0.0));
    std::vector<std::vector<double>> Hy(Nx, std::vector<double>(Ny, 0.0));

    std::vector<std::vector<double>> Ez2_sum(Nx, std::vector<double>(Ny, 0.0));
    int Ez2_count = 0;

    // -------- Simple PML damping --------
    std::vector<double> damp_x(Nx, 1.0), damp_y(Ny, 1.0);
    const int pml = 40;

    for (int i = 0; i < pml; ++i) {
        double d = std::exp(-0.02 * (pml - i));
        damp_x[i] = damp_x[Nx - 1 - i] = d;
    }
    for (int j = 0; j < pml; ++j) {
        double d = std::exp(-0.02 * (pml - j));
        damp_y[j] = damp_y[Ny - 1 - j] = d;
    }

    // -------- Dual-slit PEC screen --------
    const int screen_x = 600;
    const int slit_width = 60;
    const int slit_sep = 200;

    const int s1 = Ny / 2 - slit_sep / 2;
    const int s2 = Ny / 2 + slit_sep / 2;

    std::vector<std::vector<bool>> isPEC(
        Nx, std::vector<bool>(Ny, false));

    for (int j = 0; j < Ny; ++j) {
        bool slit1 = std::abs(j - s1) <= slit_width / 2;
        bool slit2 = std::abs(j - s2) <= slit_width / 2;
        if (!(slit1 || slit2))
            isPEC[screen_x][j] = true;
    }

    // -------- Main FDTD loop --------
    for (int n = 0; n < Nt; ++n) {

        // --- Update H ---
        for (int i = 0; i < Nx - 1; ++i)
            for (int j = 0; j < Ny - 1; ++j) {
                Hx[i][j] -= (dt / mu0) *
                    (Ez[i][j + 1] - Ez[i][j]) / dy;
                Hy[i][j] += (dt / mu0) *
                    (Ez[i + 1][j] - Ez[i][j]) / dx;
            }

        // --- Update E ---
        for (int i = 1; i < Nx - 1; ++i)
            for (int j = 1; j < Ny - 1; ++j) {
                if (isPEC[i][j]) {
                    Ez[i][j] = 0.0;
                }
                else {
                    Ez[i][j] += (dt / eps0) *
                        ((Hy[i][j] - Hy[i - 1][j]) / dx -
                            (Hx[i][j] - Hx[i][j - 1]) / dy);
                }
            }

        // --- Periodic plane wave source (right -> left) ---
        for (int j = 0; j < Ny; ++j)
            Ez[Nx - pml - 2][j] += std::sin(omega * n * dt);

        // --- Apply damping ---
        for (int i = 0; i < Nx; ++i)
            for (int j = 0; j < Ny; ++j)
                Ez[i][j] *= damp_x[i] * damp_y[j];

        // --- Intensity accumulation (far field only) ---
        if (n > 2000) {
            for (int i = screen_x + 300; i < Nx - pml; ++i)
                for (int j = 0; j < Ny; ++j)
                    Ez2_sum[i][j] += Ez[i][j] * Ez[i][j];
            Ez2_count++;
        }

        if (n % 10 == 0)
            save_png(Ez, Nx, Ny, std::string("Ez_") + std::to_string(n) + ".png");

        if (n % 500 == 0)
            printf("Step %d / %d\n", n, Nt);
    }

    // -------- Compute average intensity --------
    std::vector<std::vector<double>> I(Nx, std::vector<double>(Ny, 0.0));

    for (int i = 0; i < Nx; ++i)
        for (int j = 0; j < Ny; ++j)
            I[i][j] = std::log(1.0 + Ez2_sum[i][j] /
                std::max(1, Ez2_count));

    save_png(I, Nx, Ny, "intensity.png");

    printf("Saved intensity.png\n");
    return 0;
}
