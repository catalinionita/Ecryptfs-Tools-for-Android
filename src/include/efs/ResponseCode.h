/*
 * Copyright (C) 2008 The Android Open Source Project
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
 */

#ifndef _RESPONSECODE_H
#define _RESPONSECODE_H

class ResponseCode {
public:
    // 200 series - Requested action has been successfully completed
    static const int CommandOkay        = 200;

    // 500 series - The command was not accepted and the requested
    // action did not take place.
    static const int CommandSyntaxError = 500;
    static const int CommandNoPermission = 502;
};
#endif
