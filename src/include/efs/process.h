/**
 * Copyright (C) 2013 Intel Corporation, All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Catalin Ionita <catalin.ionita@intel.com>
 */

#ifndef EFS_PROCESS_H
#define EFS_PROCESS_H

void getProcessName(int pid, char *buffer, size_t max);
int pathMatchesMountPoint(const char *path, const char *mountPoint);
void killProcessesWithOpenFiles(const char *path, int action);

#endif /* EFS_PROCESS_H */
