#pragma once
#include <iostream>

// Centralised logging macros. Usage:
//   LOG_ERROR("Could not open " << filename);
//   LOG_INFO("Loaded " << count << " vertices");
#define LOG_ERROR(msg) (std::cerr << "[ERROR] " << msg << '\n')
#define LOG_WARN(msg)  (std::cerr << "[WARN] "  << msg << '\n')
#define LOG_INFO(msg)  (std::cout << "[INFO] "  << msg << '\n')
