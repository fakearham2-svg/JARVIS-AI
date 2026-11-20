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
 llama_context_params cp=llama_context_default_params(); cp.n_ctx=contextSize>0?contextSize:2048; cp.n_batch=256; cp.n_threads=std::max(2,(int)std::thread::hardware_concurrency()); cp.n_threads_batch=cp.n_threads;
 ctx=llama_init_from_model(model,cp); if(!ctx){llama_model_free(model);model=nullptr;return JNI_FALSE;} return JNI_TRUE;
}
extern "C" JNIEXPORT void JNICALL Java_com_jarvisai_assistant_JarvisLocalModelsPlugin_nativeUnload(JNIEnv*,jclass){std::lock_guard<std::mutex> g(lock);if(ctx){llama_free(ctx);ctx=nullptr;}if(model){llama_model_free(model);model=nullptr;}}
extern "C" JNIEXPORT jstring JNICALL Java_com_jarvisai_assistant_JarvisLocalModelsPlugin_nativeGenerate(JNIEnv* env,jclass,jstring prompt,jint maxTokens){
 std::lock_guard<std::mutex> g(lock); if(!ctx||!model)return env->NewStringUTF("ERROR: No model loaded");
 const char* p=env->GetStringUTFChars(prompt,nullptr); std::string input(p);env->ReleaseStringUTFChars(prompt,p);
 const llama_vocab* vocab=llama_model_get_vocab(model);std::vector<llama_token> tokens(input.size()+512);
 int n=llama_tokenize(vocab,input.c_str(),(int)input.size(),tokens.data(),(int)tokens.size(),true,true);
 if(n<0){tokens.resize(-n);n=llama_tokenize(vocab,input.c_str(),(int)input.size(),tokens.data(),(int)tokens.size(),true,true);} if(n<=0)return env->NewStringUTF("ERROR: Tokenization failed");tokens.resize(n);
 llama_memory_clear(llama_get_memory(ctx),true);
 llama_batch batch=llama_batch_get_one(tokens.data(),(int)tokens.size());if(llama_decode(ctx,batch)!=0)return env->NewStringUTF("ERROR: Prompt decode failed");
 llama_sampler* sampler=llama_sampler_chain_init(llama_sampler_chain_default_params());llama_sampler_chain_add(sampler,llama_sampler_init_temp(0.7f));llama_sampler_chain_add(sampler,llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
 std::string output;int limit=maxTokens>0?maxTokens:256;
 for(int i=0;i<limit;i++){llama_token t=llama_sampler_sample(sampler,ctx,-1);if(llama_vocab_is_eog(vocab,t))break;char piece[256];int len=llama_token_to_piece(vocab,t,piece,sizeof(piece),0,true);if(len>0)output.append(piece,len);llama_batch next=llama_batch_get_one(&t,1);if(llama_decode(ctx,next)!=0)break;}
 llama_sampler_free(sampler);return env->NewStringUTF(output.c_str());
}
