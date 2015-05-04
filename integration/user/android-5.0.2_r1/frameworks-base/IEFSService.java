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

package android.os.storage;

import android.os.Binder;
import android.os.IBinder;
import android.os.IInterface;
import android.os.Parcel;
import android.os.RemoteException;

public interface IEFSService extends IInterface {
    /** Local-side IPC implementation stub class. */
    public static abstract class Stub extends Binder implements IEFSService {
        private static class Proxy implements IEFSService {
            private final IBinder mRemote;

            Proxy(IBinder remote) {
                mRemote = remote;
            }

            public IBinder asBinder() {
                return mRemote;
            }

            public String getInterfaceDescriptor() {
                return DESCRIPTOR;
            }

            /**
             * Create EFS Storage
             */
            public int createEfsStorage(String storagePath, String password)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(storagePath);
                    _data.writeString(password);
                    mRemote.transact(Stub.TRANSACTION_createEfsStorage, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * Unlock EFS storage
             */
            public int unlockEfsStorage(String storagePath, String password)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(storagePath);
                    _data.writeString(password);
                    mRemote.transact(Stub.TRANSACTION_unlockEfsStorage, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * Lock EFS Storage
             */
            public int lockEfsStorage(String storagePath)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(storagePath);
                    mRemote.transact(Stub.TRANSACTION_lockEfsStorage, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * Change EFS  storage passwd
             */
            public int changePasswordEfsStorage(String storagePath, String oldPassword, String newPassword)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(storagePath);
                    _data.writeString(oldPassword);
                    _data.writeString(newPassword);
                    mRemote.transact(Stub.TRANSACTION_changePasswordEfsStorage, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }


            /**
             * Remove EFS Storage
             */
            public int removeEfsStorage(String storagePath)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(storagePath);
                    mRemote.transact(Stub.TRANSACTION_removeEfsStorage, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * get EFS Storage status
             */
            public int getEfsStorageStat(String storagePath)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(storagePath);
                    mRemote.transact(Stub.TRANSACTION_getEfsStorageStat, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * recover EFS storage encrypted data and remove storage
             */
            public int recoverDataAndRemoveEfsStorage(String storagePath, String password)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(storagePath);
                    _data.writeString(password);
                    mRemote.transact(Stub.TRANSACTION_recoverDataAndRemoveEfsStorage, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * get encryption progress
             */
            public int getEfsEncryptionProgress(String storagePath)
	                throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeString(storagePath);
                    mRemote.transact(Stub.TRANSACTION_getEfsEncryptionProgress, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
			}
			
    		/**
             * Encrypt user data
             */
            public int encryptUserData(int userId, String password)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeInt(userId);
                    _data.writeString(password);
                    mRemote.transact(Stub.TRANSACTION_encryptUserData, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * unlock user data
             */
            public int unlockUserData(int fromInit, int userId, String password)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeInt(fromInit);
                    _data.writeInt(userId);
                    _data.writeString(password);
                    mRemote.transact(Stub.TRANSACTION_unlockUserData, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * Lock user data
             */
            public int lockUserData(int userId)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeInt(userId);
                    mRemote.transact(Stub.TRANSACTION_lockUserData, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * Change user data password
             */
            public int changeUserDataPassword(int userId, String oldPassword, String newPassword)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeInt(userId);
                    _data.writeString(oldPassword);
                    _data.writeString(newPassword);
                    mRemote.transact(Stub.TRANSACTION_changeUserDataPassword, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }


            /**
             * Remove user encrypted data
             */
            public int removeUserEncryptedData(int userId)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeInt(userId);
                    mRemote.transact(Stub.TRANSACTION_removeUserEncryptedData, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * Get user encrypted data status
             */
            public int getUserEncryptedDataStat(int userId)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeInt(userId);
                    mRemote.transact(Stub.TRANSACTION_getUserEncryptedDataStat, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            /**
             * decrypt user data
             */
            public int decryptUserData(int userId, String password)
				throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeInt(userId);
                    _data.writeString(password);
                    mRemote.transact(Stub.TRANSACTION_decryptUserData, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
        }

        private static final String DESCRIPTOR = "IEFSService";

        static final int TRANSACTION_createEfsStorage = IBinder.FIRST_CALL_TRANSACTION + 0;

        static final int TRANSACTION_unlockEfsStorage = IBinder.FIRST_CALL_TRANSACTION + 1;

        static final int TRANSACTION_lockEfsStorage = IBinder.FIRST_CALL_TRANSACTION + 2;

        static final int TRANSACTION_changePasswordEfsStorage = IBinder.FIRST_CALL_TRANSACTION + 3;

        static final int TRANSACTION_removeEfsStorage = IBinder.FIRST_CALL_TRANSACTION + 4;

        static final int TRANSACTION_getEfsStorageStat = IBinder.FIRST_CALL_TRANSACTION + 5;

        static final int TRANSACTION_recoverDataAndRemoveEfsStorage = IBinder.FIRST_CALL_TRANSACTION + 6;

        static final int TRANSACTION_getEfsEncryptionProgress = IBinder.FIRST_CALL_TRANSACTION + 7;
        
        static final int TRANSACTION_encryptUserData = IBinder.FIRST_CALL_TRANSACTION + 8;

        static final int TRANSACTION_unlockUserData = IBinder.FIRST_CALL_TRANSACTION + 9;

        static final int TRANSACTION_lockUserData = IBinder.FIRST_CALL_TRANSACTION + 10;

        static final int TRANSACTION_changeUserDataPassword= IBinder.FIRST_CALL_TRANSACTION + 11;

        static final int TRANSACTION_removeUserEncryptedData = IBinder.FIRST_CALL_TRANSACTION + 12;

        static final int TRANSACTION_decryptUserData = IBinder.FIRST_CALL_TRANSACTION + 13;

        static final int TRANSACTION_getUserEncryptedDataStat = IBinder.FIRST_CALL_TRANSACTION + 14;

        /**
         * Cast an IBinder object into an IMountService interface, generating a
         * proxy if needed.
         */
        public static IEFSService asInterface(IBinder obj) {
            if (obj == null) {
                return null;
            }
            IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
            if (iin != null && iin instanceof IEFSService) {
                return (IEFSService) iin;
            }
            return new IEFSService.Stub.Proxy(obj);
        }

        /** Construct the stub at attach it to the interface. */
        public Stub() {
            attachInterface(this, DESCRIPTOR);
        }

        public IBinder asBinder() {
            return this;
        }

        @Override
        public boolean onTransact(int code, Parcel data, Parcel reply,
                int flags) throws RemoteException {
            switch (code) {
                case INTERFACE_TRANSACTION: {
                    reply.writeString(DESCRIPTOR);
                    return true;
                }
                case TRANSACTION_createEfsStorage: {
                    data.enforceInterface(DESCRIPTOR);
                    String storagePath;
                    storagePath = data.readString();
                    String password;
                    password = data.readString();
                    int resultCode = createEfsStorage(storagePath, password);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_unlockEfsStorage: {
                    data.enforceInterface(DESCRIPTOR);
                    String storagePath;
                    storagePath = data.readString();
                    String password;
                    password = data.readString();
                    int resultCode = unlockEfsStorage(storagePath, password);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_lockEfsStorage: {
                    data.enforceInterface(DESCRIPTOR);
                    String storagePath;
                    storagePath = data.readString();
                    int resultCode = lockEfsStorage(storagePath);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_changePasswordEfsStorage: {
                    data.enforceInterface(DESCRIPTOR);
                    String storagePath;
                    storagePath = data.readString();
                    String oldPassword;
                    oldPassword = data.readString();
                    String newPassword;
                    newPassword = data.readString();
                    int resultCode = changePasswordEfsStorage(storagePath, oldPassword, newPassword);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_removeEfsStorage: {
                    data.enforceInterface(DESCRIPTOR);
                    String storagePath;
                    storagePath = data.readString();
                    int resultCode = removeEfsStorage(storagePath);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_recoverDataAndRemoveEfsStorage: {
                    data.enforceInterface(DESCRIPTOR);
                    String storagePath;
                    storagePath = data.readString();
                    String password;
                    password = data.readString();
                    int resultCode = recoverDataAndRemoveEfsStorage(storagePath, password);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_getEfsStorageStat: {
                    data.enforceInterface(DESCRIPTOR);
                    String storagePath;
                    storagePath = data.readString();
                    int resultCode = getEfsStorageStat(storagePath);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_getEfsEncryptionProgress: {
                    data.enforceInterface(DESCRIPTOR);
                    String storagePath;
                    storagePath = data.readString();
                    int resultCode = getEfsEncryptionProgress(storagePath);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_encryptUserData: {
                    data.enforceInterface(DESCRIPTOR);
                    int userId;
                    userId = data.readInt();
                    String password;
                    password = data.readString();
                    int resultCode = encryptUserData(userId, password);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                return true;
                }
                case TRANSACTION_unlockUserData: {
                    data.enforceInterface(DESCRIPTOR);
                    int fromInit;
                    fromInit = data.readInt();
                    int userId;
                    userId = data.readInt();
                    String password;
                    password = data.readString();
                    int resultCode = unlockUserData(fromInit, userId, password);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_lockUserData: {
                    data.enforceInterface(DESCRIPTOR);
                    int userId;
                    userId = data.readInt();
                    int resultCode = lockUserData(userId);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_changeUserDataPassword: {
                    data.enforceInterface(DESCRIPTOR);
                    int userId;
                    userId = data.readInt();
                    String oldPassword;
                    oldPassword = data.readString();
                    String newPassword;
                    newPassword = data.readString();
                    int resultCode = changeUserDataPassword(userId, oldPassword, newPassword);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_removeUserEncryptedData: {
                    data.enforceInterface(DESCRIPTOR);
                    int userId;
                    userId = data.readInt();
                    int resultCode = removeUserEncryptedData(userId);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_getUserEncryptedDataStat: {
                    data.enforceInterface(DESCRIPTOR);
                    int userId;
                    userId = data.readInt();
                    int resultCode = getUserEncryptedDataStat(userId);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
                case TRANSACTION_decryptUserData: {
                    data.enforceInterface(DESCRIPTOR);
                    int userId;
                    userId = data.readInt();
                    String password;
                    password = data.readString();
                    int resultCode = decryptUserData(userId, password);
                    reply.writeNoException();
                    reply.writeInt(resultCode);
                    return true;
                }
            }
            return super.onTransact(code, data, reply, flags);
        }
    }

    /*
     * Create EFS storage
     */
    public int createEfsStorage(String storagePath, String password)
		throws RemoteException;

    /*
     * Unlock EFS storage
     */
    public int unlockEfsStorage(String storagePath, String password)
		throws RemoteException;

    /*
     * Lock EFS storage
     */
    public int lockEfsStorage(String storagePath)
		throws RemoteException;

    /*
     * Change EFS storage password
     */
    public int changePasswordEfsStorage(String storagePath, String oldPassword, String newPassword)
		throws RemoteException;

    /*
     * Recover EFS storage data and remove EFS storage
     */
    public int recoverDataAndRemoveEfsStorage(String storagePath, String password)
		throws RemoteException;

    /*
     * Remove EFS storage
     */
    public int removeEfsStorage(String storagePath)
		throws RemoteException;

    /*
     * get EFS storage status
     */
    public int getEfsStorageStat(String storagePath)
		throws RemoteException;

	/*
	 * get EFS encryption progress
	 */
	public int getEfsEncryptionProgress(String storagePath)
		throws RemoteException;
		
	/*
     * Encrypt user data
     */
    public int encryptUserData(int userId, String password)
	throws RemoteException;

    /*
     * Unlock user data
     */
    public int unlockUserData(int fromInit, int userId, String password)
		throws RemoteException;

    /*
     * lock user data
     */
    public int lockUserData(int userId)
		throws RemoteException;

    /*
     * change user data password
     */
    public int changeUserDataPassword(int userId, String oldPassword, String newPassword)
		throws RemoteException;

    /*
     * remove user encrypted data
     */
    public int removeUserEncryptedData(int userId)
		throws RemoteException;

    /*
     * get user encrypted data status
     */
    public int getUserEncryptedDataStat(int userId)
		throws RemoteException;

    /*
     * decrypt user data
     */
    public int decryptUserData(int userId, String password)
		throws RemoteException;
}
