#!/bin/bash
git apply vendor/intel/efs-tools/android_integration/vold/Vold-integration.patch --directory=system/vold/
git apply vendor/intel/efs-tools/android_integration/core/Core-integration.patch --directory=system/core/
git apply vendor/intel/efs-tools/android_integration/frameworks-base/MountManagerService-integration.patch --directory=frameworks/base/
git apply vendor/intel/efs-tools/android_integration/settings/Settings-integration.patch --directory=packages/apps/Settings/
