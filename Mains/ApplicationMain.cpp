#include <iostream>
#include <vector>
#include <cmath>

#include "gtc/constants.hpp"

constexpr double M_PI = glm::pi<double>();

#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Physical constants
const double c0 = 299792458.0;
const double eps0 = 8.854e-12;
const double mu0 = 4.0 * M_PI * 1e-7;

// Plane-wave source (Gaussian pulse)
double plane_wave_pulse(int n) {
    double t0 = 50.0;
    double spread = 15.0;
    return std::exp(-0.5 * std::pow((n - t0) / spread, 2));
}

double plane_wave_sine(int n, double dt) {
    const double f0 = 2.0e9;              // frequency (Hz)
    const double omega = 2.0 * M_PI * f0;

    // Smooth turn-on to avoid wideband noise
    const double t_ramp = 20.0 / f0;
    double t = n * dt;

    double ramp = (t < t_ramp)
        ? 0.5 * (1.0 - std::cos(M_PI * t / t_ramp))
        : 1.0;

    return ramp * std::sin(omega * t);
}

// Save Ez field to PNG
void save_png(const std::vector<std::vector<double>>& Ez,
    int Nx, int Ny, int step)
{
    std::vector<unsigned char> img(Nx * Ny);

    double maxv = 0.0;
    for (int i = 0; i < Nx; ++i)
        for (int j = 0; j < Ny; ++j)
            maxv = std::max(maxv, std::abs(Ez[i][j]));
    if (maxv == 0.0) maxv = 1.0;

    for (int j = 0; j < Ny; ++j)
        for (int i = 0; i < Nx; ++i) {
            double v = Ez[i][j] / maxv;
            v = 0.5 * (v + 1.0);
            img[j * Nx + i] = (unsigned char)(255.0 * v);
        }

    char name[64];
    std::snprintf(name, sizeof(name), "Ez_%04d.png", step);
    stbi_write_png(name, Nx, Ny, 1, img.data(), Nx);
}

int main() {
    // Grid
    const int Nx = 400;
    const int Ny = 400;
    const int Nt = 1200;
    const int pml = 10;

    const double dx = 1e-3;
    const double dy = 1e-3;
    const double dt = 1.0 / (c0 * std::sqrt(1.0 / (dx * dx) + 1.0 / (dy * dy)));

    // Fields
    std::vector<std::vector<double>> Ezx(Nx, std::vector<double>(Ny, 0.0));
    std::vector<std::vector<double>> Ezy(Nx, std::vector<double>(Ny, 0.0));
    std::vector<std::vector<double>> Ez(Nx, std::vector<double>(Ny, 0.0));
    std::vector<std::vector<double>> Hx(Nx, std::vector<double>(Ny, 0.0));
    std::vector<std::vector<double>> Hy(Nx, std::vector<double>(Ny, 0.0));

    // PML conductivity
    std::vector<double> sigma_x(Nx, 0.0), sigma_y(Ny, 0.0);
    double sigma_max = 1.0;
    int m = 3;

    for (int i = 0; i < pml; ++i) {
        double s = std::pow((pml - i) / (double)pml, m);
        sigma_x[i] = sigma_x[Nx - 1 - i] = sigma_max * s;
        sigma_y[i] = sigma_y[Ny - 1 - i] = sigma_max * s;
    }

    std::vector<double> ax(Nx), bx(Nx), ay(Ny), by(Ny);
    for (int i = 0; i < Nx; ++i) {
        ax[i] = (1.0 - sigma_x[i] * dt / (2 * eps0)) /
            (1.0 + sigma_x[i] * dt / (2 * eps0));
        bx[i] = (dt / eps0) /
            (1.0 + sigma_x[i] * dt / (2 * eps0));
    }
    for (int j = 0; j < Ny; ++j) {
        ay[j] = (1.0 - sigma_y[j] * dt / (2 * eps0)) /
            (1.0 + sigma_y[j] * dt / (2 * eps0));
        by[j] = (dt / eps0) /
            (1.0 + sigma_y[j] * dt / (2 * eps0));
    }

    // TF/SF boundary (right side)
    const int tf_x = Nx - pml - 2;

    // Time loop
    for (int n = 0; n < Nt; ++n) {

        // Update H
        for (int i = 0; i < Nx; ++i)
            for (int j = 0; j < Ny - 1; ++j)
                Hx[i][j] -= (dt / (mu0 * dy)) * (Ez[i][j + 1] - Ez[i][j]);

        for (int i = 0; i < Nx - 1; ++i)
            for (int j = 0; j < Ny; ++j)
                Hy[i][j] += (dt / (mu0 * dx)) * (Ez[i + 1][j] - Ez[i][j]);

        // TF/SF correction (Hy)
        double src = plane_wave_sine(n, dt);
        for (int j = 1; j < Ny - 1; ++j)
            Hy[tf_x][j] -= (dt / (mu0 * dx)) * src;

        // Update Ez (split)
        for (int i = 1; i < Nx - 1; ++i)
            for (int j = 1; j < Ny - 1; ++j) {
                Ezx[i][j] = ax[i] * Ezx[i][j]
                    + bx[i] * (Hy[i][j] - Hy[i - 1][j]) / dx;

                Ezy[i][j] = ay[j] * Ezy[i][j]
                    - by[j] * (Hx[i][j] - Hx[i][j - 1]) / dy;

                Ez[i][j] = Ezx[i][j] + Ezy[i][j];
            }

        // TF/SF correction (Ez)
        for (int j = 1; j < Ny - 1; ++j)
            Ez[tf_x - 1][j] += src;



        // Output
        if (n % 10 == 0)
            save_png(Ez, Nx, Ny, n);
    }

    return 0;
}
       