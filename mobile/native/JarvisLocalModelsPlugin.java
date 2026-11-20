package com.jarvisai.assistant;

import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.os.StatFs;
import android.provider.OpenDocument;
import android.database.Cursor;
import android.provider.OpenableColumns;
import java.io.*;
import java.util.*;
import org.json.JSONArray;
import org.json.JSONObject;
import com.getcapacitor.*;

@CapacitorPlugin(name = "JarvisLocalModels")
public class JarvisLocalModelsPlugin extends Plugin {
  @PluginMethod public void pickModel(PluginCall call) {
    Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
    intent.setType("*/*");
    intent.addCategory(Intent.CATEGORY_OPENABLE);
    intent.putExtra(Intent.EXTRA_MIME_TYPES, new String[]{"application/octet-stream","application/x-gguf"});
    startActivityForResult(call, intent, "modelPicked");
  }
  @ActivityCallback private void modelPicked(PluginCall call, ActivityResult result) {
    if (result.getResultCode()!=android.app.Activity.RESULT_OK || result.getData()==null) { call.reject("Import cancelled"); return; }
    Uri uri=result.getData().getData();
    if(uri==null){call.reject("No file selected");return;}
    String name="model.gguf";
    try(Cursor c=getContext().getContentResolver().query(uri,null,null,null,null)){
      if(c!=null&&c.moveToFirst()){int n=c.getColumnIndex(OpenableColumns.DISPLAY_NAME);if(n>=0)name=c.getString(n);}
    }catch(Exception ignored){}
    if(!name.toLowerCase(Locale.ROOT).endsWith(".gguf")){call.reject("Please select a .gguf model file");return;}
    File dir=new File(getContext().getFilesDir(),"gguf-models");dir.mkdirs();
    File target=new File(dir,name); if(target.exists()) target=new File(dir,System.currentTimeMillis()+"-"+name);
    try(InputStream in=getContext().getContentResolver().openInputStream(uri);OutputStream out=new FileOutputStream(target)){
      if(in==null)throw new IOException("Unable to read selected file");
      byte[] b=new byte[1024*1024];int k;while((k=in.read(b))!=-1)out.write(b,0,k);
      JSObject o=new JSObject();o.put("name",target.getName());o.put("size",target.length());o.put("id",target.getName());call.resolve(o);
    }catch(Exception e){call.reject("Import failed: "+e.getMessage());}
  }
  @PluginMethod public void listModels(PluginCall call){
    File dir=new File(getContext().getFilesDir(),"gguf-models");dir.mkdirs();JSONArray a=new JSONArray();
    File[] files=dir.listFiles();if(files!=null)for(File f:files)if(f.isFile()&&f.getName().toLowerCase(Locale.ROOT).endsWith(".gguf")){JSONObject o=new JSONObject();try{o.put("name",f.getName());o.put("id",f.getName());o.put("size",f.length());o.put("path",f.getAbsolutePath());a.put(o);}catch(Exception ignored){}}
    JSObject r=new JSObject();r.put("models",a);StatFs fs=new StatFs(getContext().getFilesDir().getAbsolutePath());r.put("availableBytes",fs.getAvailableBytes());r.put("totalBytes",fs.getTotalBytes());call.resolve(r);
  }
  @PluginMethod public void removeModel(PluginCall call){String id=call.getString("id");if(id==null||id.contains("/")||id.contains("\\")||id.equals("..")){call.reject("Invalid model id");return;}File f=new File(new File(getContext().getFilesDir(),"gguf-models"),id);if(!f.exists()||!f.delete()){call.reject("Could not remove model");return;}call.resolve();}
}