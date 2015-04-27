/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.android.server;

import android.content.Context;
import android.os.HandlerThread;
import android.os.storage.IEFSService;
import android.os.storage.StorageResultCode;
import android.util.Log;

public class EFSService extends IEFSService.Stub
        implements INativeDaemonConnectorCallbacks {

    static EFSService sSelf = null;

    private static final String TAG = "EfsService";

    private static final String EFS_TAG = "EfsServiceConnector";

    /** Maximum number of ASEC containers allowed to be mounted. */
    private static final int MAX_CONTAINERS = 250;
    
    private final Context mContext;
    private final NativeDaemonConnector mConnector;

    private static final int OpFailedStorageBusy = 405;
    
    /**
     * Callback from NativeDaemonConnector
     */
    public void onDaemonConnected() {
    }

    /**
     * Callback from NativeDaemonConnector
     */
    public boolean onCheckHoldWakeLock(int code) {
        return false;
    }

    /**
     * Callback from NativeDaemonConnector
     */
    public boolean onEvent(int code, String raw, String[] cooked) {
       return true;
    }
    
    public EFSService(Context context) {
        sSelf = this;

        mContext = context;

        mConnector = new NativeDaemonConnector(this, "efs-server", MAX_CONTAINERS * 2, EFS_TAG, 25,
                null);

        Thread thread = new Thread(mConnector, EFS_TAG);
        thread.start();
    }
    
    public int createEfsStorage(String storagePath, String password) {
        int rc = StorageResultCode.OperationSucceeded;
        final NativeDaemonEvent event;
        try {
            event = mConnector.execute("efs-server", "create", storagePath, password);
            return Integer.parseInt(event.getMessage());
        } catch (NativeDaemonConnectorException e) {
            int code = e.getCode();
            if (code != OpFailedStorageBusy) {
                rc = StorageResultCode.OperationFailedInternalError;
            }
        }
        return rc;
    }

    public int unlockEfsStorage(String storagePath, String password) {
        int rc = StorageResultCode.OperationSucceeded;
        final NativeDaemonEvent event;
        try {
            event = mConnector.execute("efs-server", "unlock", storagePath, password);
            return Integer.parseInt(event.getMessage());
        } catch (NativeDaemonConnectorException e) {
            int code = e.getCode();
            if (code != OpFailedStorageBusy) {
                rc = StorageResultCode.OperationFailedInternalError;
            }
        }
        return rc;
    }


    public int lockEfsStorage(String storagePath) {
        int rc = StorageResultCode.OperationSucceeded;
        final NativeDaemonEvent event;
        try {
            event = mConnector.execute("efs-server", "lock", storagePath);
            return Integer.parseInt(event.getMessage());
        } catch (NativeDaemonConnectorException e) {
            int code = e.getCode();
            if (code != OpFailedStorageBusy) {
                rc = StorageResultCode.OperationFailedInternalError;
            }
        }
        return rc;
    }

    public int changePasswordEfsStorage(String storagePath, String oldPassword, String newPassword) {
        int rc = StorageResultCode.OperationSucceeded;
        final NativeDaemonEvent event;
        try {
            event = mConnector.execute("efs-server", "change_passwd", storagePath, oldPassword, newPassword);
            return Integer.parseInt(event.getMessage());
        } catch (NativeDaemonConnectorException e) {
            int code = e.getCode();
            if (code != OpFailedStorageBusy) {
                rc = StorageResultCode.OperationFailedInternalError;
            }
        }
        return rc;
    }

    public int removeEfsStorage(String storagePath) {
        int rc = StorageResultCode.OperationSucceeded;
        final NativeDaemonEvent event;
        try {
            event = mConnector.execute("efs-server", "remove", storagePath);
            return Integer.parseInt(event.getMessage());
        } catch (NativeDaemonConnectorException e) {
            int code = e.getCode();
            if (code != OpFailedStorageBusy) {
                rc = StorageResultCode.OperationFailedInternalError;
            }
        }
        return rc;
    }

    public int getEfsStorageStat(String storagePath) {
        int rc = StorageResultCode.OperationSucceeded;
        final NativeDaemonEvent event;
        try {
            event = mConnector.execute("efs-server", "stat", storagePath);
            return Integer.parseInt(event.getMessage());
        } catch (NativeDaemonConnectorException e) {
            int code = e.getCode();
            if (code != OpFailedStorageBusy) {
                rc = StorageResultCode.OperationFailedInternalError;
            }
        }
        return rc;
    }

    public int recoverDataAndRemoveEfsStorage(String storagePath, String password) {
        int rc = StorageResultCode.OperationSucceeded;
        final NativeDaemonEvent event;
        try {
            event = mConnector.execute("efs-server", "recover", storagePath, password);
            return Integer.parseInt(event.getMessage());
        } catch (NativeDaemonConnectorException e) {
            int code = e.getCode();
            if (code != OpFailedStorageBusy) {
                rc = StorageResultCode.OperationFailedInternalError;
            }
        }
        return rc;
    }
    
    public int getEfsEncryptionProgress(String storagePath) {
        int rc = StorageResultCode.OperationSucceeded;
        final NativeDaemonEvent event;
        try {
            event = mConnector.execute("efs-server", "get_progress", storagePath);
            return Integer.parseInt(event.getMessage());
        } catch (NativeDaemonConnectorException e) {
            int code = e.getCode();
            if (code != OpFailedStorageBusy) {
                rc = StorageResultCode.OperationFailedInternalError;
            }
        }
        return rc;
    }
}

