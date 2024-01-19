#pragma once

#define VERSION_TO_STRING_HELPER(v) #v
#define VERSION_TO_STRING(v) VERSION_TO_STRING_HELPER(v)

#define PRAXIS3D_COMMIT_DATE 2024-01-19

#define PRAXIS3D_VERSION_MAJOR 0
#define PRAXIS3D_VERSION_MINOR 2
#define PRAXIS3D_VERSION_PATCH 1

#define PRAXIS3D_VERSION ((PRAXIS3D_VERSION_MAJOR << 16) | (PRAXIS3D_VERSION_MINOR << 8) | PRAXIS3D_VERSION_PATCH)
#define PRAXIS3D_VERSION_STRING VERSION_TO_STRING(PRAXIS3D_VERSION_MAJOR) "." VERSION_TO_STRING(PRAXIS3D_VERSION_MINOR) "." VERSION_TO_STRING(PRAXIS3D_VERSION_PATCH)