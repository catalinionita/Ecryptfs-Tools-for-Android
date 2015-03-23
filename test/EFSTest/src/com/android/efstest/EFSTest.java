package com.android.efstest;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ServiceManager;
import android.os.storage.IEFSService;
import android.os.SELinux;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import android.util.Log;

import java.io.File;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileOutputStream;
import java.util.LinkedHashMap;
import java.util.Map;

public class EFSTest extends Activity {

    private static final String TAG = "EFSTest";

    private String STORAGE_PATH = "/new_folder";
    private String ENCRYPTED_STORAGE_PATH = "/.new_folder";
    private final String OLD_PASS = "testold";
    private final String NEW_PASS = "testnew";
    
    private IEFSService ms;
    
    private void createFile(String path, String content) {
        try (FileOutputStream outputStream = new FileOutputStream(path)) {
            outputStream.write(content.getBytes());
        } catch (Exception e) {
            Log.e(TAG, "Error writing to file ", e);
        }
    }
    
    private void delete(File f) {
        try {
            if (f.exists() && f.isDirectory() && f.listFiles() != null) {
                for (File sf : f.listFiles())
                    delete(sf);
            }
            f.delete();
        } catch (Exception e) {
            Log.e(TAG, "Error deleting file ", e);
        }
    }
    
    private boolean checkFile(String path, String content) {
        try (BufferedReader br = new BufferedReader(new FileReader(path))) {
            String line = null;
            String input = "";
            while ((line = br.readLine()) != null) {
                input += line + "\n";
            }
            
            return input.equals(content);
        } catch (Exception e) {
            Log.e(TAG, "Error reading from file ", e);
            return false;
        }
    }

    private abstract class Tester {
        public boolean test() {
            return true;
        }
    }

    private class CreateTester extends Tester {
        private String path, encryptedPath, pass;

        public CreateTester(String path, String encryptedPath, String pass) {
            this.path = path;
            this.encryptedPath = encryptedPath;
            this.pass = pass;
        }

        public boolean test() {
            try {
                File f = new File(path);
                File ef = new File(encryptedPath);
                
                delete(f);
                delete(ef);
            
                if (f.mkdirs() == false) {
                    Log.e(TAG, "Failed to create dirs");
                    return false;
                }
                    
                createFile(path + "/a.txt", "aaa\n");
                createFile(path + "/b.txt", "bbb\n");
                createFile(path + "/c.txt", "ccc\n");
            
                if (ms.createEfsStorage(path, pass) != 0) {
                    Log.e(TAG, "createEfsStorage failed");
                    return false;
                }
                    
                if ((ef.exists() && ef.isDirectory()) == false) {
                    Log.e(TAG, "Storage does not exist");
                    return false;
                }

                return true;
                 
            } catch (Exception e) {
                Log.e(TAG, "Error creating storage ", e);
                return false;
            }
        }
    }
    
    private class CreateFailTester extends Tester {
        private String path, encryptedPath, pass;

        public CreateFailTester(String path, String encryptedPath, String pass) {
            this.path = path;
            this.encryptedPath = encryptedPath;
            this.pass = pass;
        }

        public boolean test() {
            try {
                File f = new File(path);
                File ef = new File(encryptedPath);
                
                delete(f);
                delete(ef);
            
                if (f.mkdirs() == false)
                    return false;
                
                if (ms.createEfsStorage(path, pass) == 0)
                    return false;
                    
                if (ef.exists())
                    return false;
                    
                return true;
                
            } catch (Exception e) {
                Log.e(TAG, "Error attempting to create storage ", e);
                return false;
            }
        }
    }

    private class UnlockTester extends Tester {
        private String path, pass;

        public UnlockTester(String path, String pass) {
            this.path = path;
            this.pass = pass;
        }

        public boolean test() {
            try {
                if (ms.unlockEfsStorage(path, pass) != 0)
                    return false;
                    
                if (checkFile(path + "/a.txt", "aaa\n") == false)
                    return false;
                if (checkFile(path + "/b.txt", "bbb\n") == false)
                    return false;
                if (checkFile(path + "/c.txt", "ccc\n") == false)
                    return false;
                    
                return true;
            } catch (Exception e) {
                Log.e(TAG, "Error unlocking storage ", e);
                return false;
            }
        }
    }

    private class LockTester extends Tester {
        private String path;

        public LockTester(String path) {
            this.path = path;
        }

        public boolean test() {
            try {
                return ms.lockEfsStorage(path) == 0;
            } catch (Exception e) {
                Log.e(TAG, "Error unlocking storage ", e);
                return false;
            }
        }
    }

    private class ChangePasswordTester extends Tester {
        private String path, oldPass, newPass;

        public ChangePasswordTester(String path, String oldPass, String newPass) {
            this.path = path;
            this.oldPass = oldPass;
            this.newPass = newPass;
        }

        public boolean test() {
            try {
                return ms.changePasswordEfsStorage(path, oldPass, newPass) == 0;
            } catch (Exception e) {
                Log.e(TAG, "Error changing password ", e);
                return false;
            }
        }
    }
    
    private class ChangePasswordFailTester extends Tester {
        private String path, oldPass, newPass;

        public ChangePasswordFailTester(String path, String oldPass, String newPass) {
            this.path = path;
            this.oldPass = oldPass;
            this.newPass = newPass;
        }

        public boolean test() {
            try {
                return ms.changePasswordEfsStorage(path, oldPass, newPass) != 0;
            } catch (Exception e) {
                Log.e(TAG, "Error changing password ", e);
                return false;
            }
        }
    }

    private class RestoreTester extends Tester {
        private String path, encryptedPath, pass;

        public RestoreTester(String path, String encryptedPath, String pass) {
            this.path = path;
            this.encryptedPath = encryptedPath;
            this.pass = pass;
        }

        public boolean test() {
            try {
                File f = new File(path);
                File ef = new File(encryptedPath);
            
                if ((f.exists() && f.isDirectory() && 
                    ef.exists() && ef.isDirectory()) == false)
                    return false;
            
                if (ms.recoverDataAndRemoveEfsStorage(path, pass) != 0)
                    return false;
                    
                if ((f.exists() && f.isDirectory()) == false)
                    return false;
                    
                if (checkFile(path + "/a.txt", "aaa\n") == false)
                    return false;
                if (checkFile(path + "/b.txt", "bbb\n") == false)
                    return false;
                if (checkFile(path + "/c.txt", "ccc\n") == false)
                    return false;
                    
                if (ef.exists())
                    return false;
                    
                return true;
                 
            } catch (Exception e) {
                Log.e(TAG, "Error restoring storage ", e);
                return false;
            }
        }
    }

    private class RemoveTester extends Tester {
        private String path, encryptedPath;

        public RemoveTester(String path, String encryptedPath) {
            this.path = path;
            this.encryptedPath = encryptedPath;
        }

        public boolean test() {
            try {
                File f = new File(path);
                File ef = new File(encryptedPath);
            
                if ((f.exists() && f.isDirectory() && 
                    ef.exists() && ef.isDirectory()) == false)
                    return false;
            
                if (ms.removeEfsStorage(path) != 0)
                    return false;
                    
                if ((f.exists() || ef.exists()) == false)
                    return false;
                    
                return true;
                 
            } catch (Exception e) {
                Log.e(TAG, "Error removing storage ", e);
                return false;
            }
        }
    }

    private Map<Tester, TextView> test2view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_efstest);
        
        IBinder service = ServiceManager.getService("efsservice");
        if (service != null)
            ms = IEFSService.Stub.asInterface(service);

        STORAGE_PATH = getFilesDir() + STORAGE_PATH;
        ENCRYPTED_STORAGE_PATH = getFilesDir() + ENCRYPTED_STORAGE_PATH;

        test2view = new LinkedHashMap<Tester, TextView>();
        test2view.put(new CreateTester(STORAGE_PATH, ENCRYPTED_STORAGE_PATH, OLD_PASS),
                (TextView) findViewById(R.id.textView2));
        test2view.put(new RemoveTester(STORAGE_PATH, ENCRYPTED_STORAGE_PATH),
                (TextView) findViewById(R.id.textView3));
        test2view.put(new CreateTester(STORAGE_PATH, ENCRYPTED_STORAGE_PATH, OLD_PASS),
                (TextView) findViewById(R.id.textView4));
        test2view.put(new UnlockTester(STORAGE_PATH, OLD_PASS),
                (TextView) findViewById(R.id.textView6));
        test2view.put(new LockTester(STORAGE_PATH),
                (TextView) findViewById(R.id.textView7));
        test2view.put(new ChangePasswordFailTester(STORAGE_PATH, OLD_PASS, "qwe"),
                (TextView) findViewById(R.id.textView8));
        test2view.put(new ChangePasswordTester(STORAGE_PATH, OLD_PASS, NEW_PASS),
                (TextView) findViewById(R.id.textView9));
        test2view.put(new UnlockTester(STORAGE_PATH, NEW_PASS),
                (TextView) findViewById(R.id.textView10));
        test2view.put(new LockTester(STORAGE_PATH),
                (TextView) findViewById(R.id.textView11));
        test2view.put(new RestoreTester(STORAGE_PATH, ENCRYPTED_STORAGE_PATH, NEW_PASS),
                (TextView) findViewById(R.id.textView));

        test2view.put(new CreateFailTester(getFilesDir() + "/bla", getFilesDir() + "/.bla", "bla"),
                (TextView) findViewById(R.id.textView12));
        test2view.put(new CreateTester(getFilesDir() + "/bla", getFilesDir() + "/.bla", "test1"),
                (TextView) findViewById(R.id.textView13));
        test2view.put(new RemoveTester(getFilesDir() + "/bla", getFilesDir() + "/.bla"),
                (TextView) findViewById(R.id.textView14));


        Button buttonOne = (Button) findViewById(R.id.button);
        buttonOne.setOnClickListener (new Button.OnClickListener() {
            public void onClick(View v) {

                for (Map.Entry<Tester, TextView> en : test2view.entrySet()) {
                    en.getValue().setTextColor(en.getKey().test() ? Color.GREEN : Color.RED);
                }
            }
        });
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.efstest, menu);
        return true;
    }
}
