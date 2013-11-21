/**
 * @file   EncryptedFileStorageCmd.h
 * @author Catalin Ionita <catalin.ionita@intel.com>
 * @date   Wed Oct 16 13:06:43 2013
 * 
 * @brief  
 * EFS comands for vold service
 * 
 */

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

class EncryptedFileStorageCmd : public VoldCommand {
 public:
	EncryptedFileStorageCmd();
	virtual ~EncryptedFileStorageCmd() {}
	int runCommand(SocketClient *c, int argc, char ** argv);
};
