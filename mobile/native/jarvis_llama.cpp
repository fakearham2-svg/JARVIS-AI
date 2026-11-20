#include <jni.h>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <algorithm>
#include "llama.h"
static llama_model * model=nullptr;
static llama_context * ctx=nullptr;
static std::mutex lock;
extern "C" JNIEXPORT jboolean JNICALL Java_com_jarvisai_assistant_JarvisLocalModelsPlugin_nativeLoad(JNIEnv* env,jclass,jstring path,jint contextSize){
 std::lock_guard<std::mutex> g(lock);
 if(ctx){llama_free(ctx);ctx=nullptr;} if(model){llama_model_free(model);model=nullptr;}
 const char* p=env->GetStringUTFChars(path,nullptr);
 llama_model_params mp=llama_model_default_params(); mp.n_gpu_layers=0;
 model=llama_model_load_from_file(p,mp); env->ReleaseStringUTFChars(path,p);
 if(!model)return JNI_FALSE;
 llama_context_params cp=llama_context_default_params(); cp.n_ctx=contextSize>0?contextSize:2048; cp.n_batch=256;
 int threads=std::max(2,(int)std::thread::hardware_concurrency()); threads=std::min(threads,4);
 cp.n_threads=threads; cp.n_threads_batch=threads;
 ctx=llama_init_from_model(model,cp); if(!ctx){llama_model_free(model);model=nullptr;return JNI_FALSE;} return JNI_TRUE;
}
extern "C" JNIEXPORT void JNICALL Java_com_jarvisai_assistant_JarvisLocalModelsPlugin_nativeUnload(JNIEnv*,jclass){std::lock_guard<std::mutex> g(lock);if(ctx){llama_free(ctx);ctx=nullptr;}if(model){llama_model_free(model);model=nullptr;}}
extern "C" JNIEXPORT jstring JNICALL Java_com_jarvisai_assistant_JarvisLocalModelsPlugin_nativeGenerate(JNIEnv* env,jclass,jstring prompt,jint maxTokens){
 std::lock_guard<std::mutex> g(lock); if(!ctx||!model)return env->NewStringUTF("ERROR: No model loaded");
 const char* p=env->GetStringUTFChars(prompt,nullptr); std::string input(p);env->ReleaseStringUTFChars(prompt,p);
 const char* system="You are JARVIS, a helpful, accurate, friendly assistant. Reply in the same language and style as the user. If the user writes Roman Urdu/Hinglish, reply naturally in Roman Urdu/Hinglish; if English, reply in English. Keep answers clear and relevant. Do not invent facts. Answer the latest user message directly. Do not repeat the prompt or output role labels.";
 llama_chat_message messages[2]={{"system",system},{"user",input.c_str()}};
 const char* modelTemplate=llama_model_chat_template(model,nullptr);
 int32_t needed=llama_chat_apply_template(modelTemplate,messages,2,true,nullptr,0);
 std::string formatted;
 if(needed>0){formatted.resize((size_t)needed+1);int32_t written=llama_chat_apply_template(modelTemplate,messages,2,true,formatted.data(),(int32_t)formatted.size());if(written>0)formatted.resize((size_t)written);else formatted.clear();}
 if(formatted.empty()){needed=llama_chat_apply_template("chatml",messages,2,true,nullptr,0);if(needed>0){formatted.resize((size_t)needed+1);int32_t written=llama_chat_apply_template("chatml",messages,2,true,formatted.data(),(int32_t)formatted.size());if(written>0)formatted.resize((size_t)written);else formatted.clear();}}
 if(formatted.empty())formatted=std::string("System: ")+system+"\nUser: "+input+"\nAssistant:";
 const llama_vocab* vocab=llama_model_get_vocab(model);
 std::vector<llama_token> tokens(formatted.size()+512);
 int n=llama_tokenize(vocab,formatted.c_str(),(int)formatted.size(),tokens.data(),(int)tokens.size(),false,true);
 if(n<0){tokens.resize(-n);n=llama_tokenize(vocab,formatted.c_str(),(int)formatted.size(),tokens.data(),(int)tokens.size(),true,true);} if(n<=0)return env->NewStringUTF("ERROR: Tokenization failed");tokens.resize(n);
 int context=(int)llama_n_ctx(ctx),limit=maxTokens>0?maxTokens:256;
 if((int)tokens.size()+limit>=context){return env->NewStringUTF("ERROR: Prompt too long for selected context. Increase context size or start a new chat.");}
 llama_memory_clear(llama_get_memory(ctx),true);
 llama_batch batch=llama_batch_get_one(tokens.data(),(int)tokens.size());if(llama_decode(ctx,batch)!=0)return env->NewStringUTF("ERROR: Prompt decode failed");
 llama_sampler* sampler=llama_sampler_chain_init(llama_sampler_chain_default_params());
 llama_sampler_chain_add(sampler,llama_sampler_init_temp(0.55f));
 llama_sampler_chain_add(sampler,llama_sampler_init_top_p(0.9f,1));
 llama_sampler_chain_add(sampler,llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
 std::string output;
 for(int i=0;i<limit;i++){llama_token t=llama_sampler_sample(sampler,ctx,-1);if(llama_vocab_is_eog(vocab,t))break;char piece[512];int len=llama_token_to_piece(vocab,t,piece,sizeof(piece),0,true);if(len>0)output.append(piece,len);llama_batch next=llama_batch_get_one(&t,1);if(llama_decode(ctx,next)!=0)break;}
 llama_sampler_free(sampler);
 if(output.empty())return env->NewStringUTF("ERROR: Model generated no text. Try a different instruct/chat GGUF model.");
 return env->NewStringUTF(output.c_str());
}
