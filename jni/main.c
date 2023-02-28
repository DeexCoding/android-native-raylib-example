#include "jni.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <android/window.h>

#include "raylib.h"
struct android_app *GetAndroidApp(void);

#define LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "CarGame", __VA_ARGS__))

#ifdef __cplusplus
#define SETUP_FOR_JAVA_CALL \
	JNIEnv * env = 0; \
	JNIEnv ** envptr = &env; \
	JavaVM * jniiptr = app->activity->vm; \
	jniiptr->AttachCurrentThread( (JNIEnv**)&env, 0 ); \
	env = (*envptr);
#define ENVCALL
#define JAVA_CALL_DETACH 	jniiptr->DetachCurrentThread();
#else
#define SETUP_FOR_JAVA_CALL \
	const struct JNINativeInterface * env = (struct JNINativeInterface*)app->activity->env; \
	const struct JNINativeInterface ** envptr = &env; \
	const struct JNIInvokeInterface ** jniiptr = app->activity->vm; \
	const struct JNIInvokeInterface * jnii = *jniiptr; \
	jnii->AttachCurrentThread(jniiptr, &envptr, 0); \
	env = (*envptr);
#define ENVCALL envptr,
#define JAVA_CALL_DETACH       	jnii->DetachCurrentThread( jniiptr );
#endif

void SetImmersiveMode(struct android_app* app) 
{
    SETUP_FOR_JAVA_CALL

    jclass activityClass = env->FindClass(envptr, "android/app/NativeActivity");
    jclass windowClass = env->FindClass(envptr, "android/view/Window");
    jclass viewClass = env->FindClass(envptr, "android/view/View");
    jmethodID getWindow = env->GetMethodID(envptr, activityClass, "getWindow", "()Landroid/view/Window;");
    jmethodID getDecorView = env->GetMethodID(envptr, windowClass, "getDecorView", "()Landroid/view/View;");
    jmethodID setSystemUiVisibility = env->GetMethodID(envptr, viewClass, "setSystemUiVisibility", "(I)V");
    jmethodID getSystemUiVisibility = env->GetMethodID(envptr, viewClass, "getSystemUiVisibility", "()I");

    jobject windowObj = env->CallObjectMethod(envptr, app->activity->clazz, getWindow);
    jobject decorViewObj = env->CallObjectMethod(envptr, windowObj, getDecorView);

    // Get flag ids
    jfieldID id_SYSTEM_UI_FLAG_LAYOUT_STABLE = env->GetStaticFieldID(envptr, viewClass, "SYSTEM_UI_FLAG_LAYOUT_STABLE", "I");
    jfieldID id_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = env->GetStaticFieldID(envptr, viewClass, "SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION", "I");
    jfieldID id_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = env->GetStaticFieldID(envptr, viewClass, "SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN", "I");
    jfieldID id_SYSTEM_UI_FLAG_HIDE_NAVIGATION = env->GetStaticFieldID(envptr, viewClass, "SYSTEM_UI_FLAG_HIDE_NAVIGATION", "I");
    jfieldID id_SYSTEM_UI_FLAG_FULLSCREEN = env->GetStaticFieldID(envptr, viewClass, "SYSTEM_UI_FLAG_FULLSCREEN", "I");
    jfieldID id_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = env->GetStaticFieldID(envptr, viewClass, "SYSTEM_UI_FLAG_IMMERSIVE_STICKY", "I");

    // Get flags
    const int flag_SYSTEM_UI_FLAG_LAYOUT_STABLE = env->GetStaticIntField(envptr, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_STABLE);
    const int flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = env->GetStaticIntField(envptr, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
    const int flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = env->GetStaticIntField(envptr, viewClass, id_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
    const int flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION = env->GetStaticIntField(envptr, viewClass, id_SYSTEM_UI_FLAG_HIDE_NAVIGATION);
    const int flag_SYSTEM_UI_FLAG_FULLSCREEN = env->GetStaticIntField(envptr, viewClass, id_SYSTEM_UI_FLAG_FULLSCREEN);
    const int flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = env->GetStaticIntField(envptr, viewClass, id_SYSTEM_UI_FLAG_IMMERSIVE_STICKY);

    // Get current immersiveness
    const int currentVisibility = env->CallIntMethod(envptr, decorViewObj, getSystemUiVisibility);
    const bool is_SYSTEM_UI_FLAG_LAYOUT_STABLE = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_STABLE) != 0;
    const bool is_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION) != 0;
    const bool is_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = (currentVisibility & flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN) != 0;
    const bool is_SYSTEM_UI_FLAG_HIDE_NAVIGATION = (currentVisibility & flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION) != 0;
    const bool is_SYSTEM_UI_FLAG_FULLSCREEN = (currentVisibility & flag_SYSTEM_UI_FLAG_FULLSCREEN) != 0;
    const bool is_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = (currentVisibility & flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY) != 0;

    const bool isAlreadyImmersive =
        is_SYSTEM_UI_FLAG_LAYOUT_STABLE &&
        is_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION &&
        is_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN &&
        is_SYSTEM_UI_FLAG_HIDE_NAVIGATION &&
        is_SYSTEM_UI_FLAG_FULLSCREEN &&
        is_SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
    {
        const int flag =
            flag_SYSTEM_UI_FLAG_LAYOUT_STABLE |
            flag_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
            flag_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
            flag_SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            flag_SYSTEM_UI_FLAG_FULLSCREEN |
            flag_SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        env->CallVoidMethod(envptr, decorViewObj, setSystemUiVisibility, flag);
        if(env->ExceptionCheck(envptr)) 
        {
            LOG("Could not set immersive mode");

            // Read exception msg
            /*jthrowable e = env->ExceptionOccurred();
            env->ExceptionClear(); // clears the exception; e seems to remain valid
            jclass clazz = env->GetObjectClass(e);
            jmethodID getMessage = env->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");
            jstring message = (jstring)env->CallObjectMethod(e, getMessage);
            const char *mstr = env->GetStringUTFChars(message, NULL);
            const auto exception_msg = std::string{mstr};
            env->ReleaseStringUTFChars(message, mstr);
            env->DeleteLocalRef(message);
            env->DeleteLocalRef(clazz);
            env->DeleteLocalRef(e);
            PORTIS_LOGW() << "set_immersive exception [" << exception_msg << "]";
            success = false;*/
        }
        else 
        {
            LOG("Set immersive mode successfully");

            //PORTIS_LOGI() << "set_immersive success";
        }
    }

    env->DeleteLocalRef(envptr, windowObj);
    env->DeleteLocalRef(envptr, decorViewObj);
    
    JAVA_CALL_DETACH
}

int main()
{
    LOG("START");

    const int screenWidth = 800;
    const int screenHeight = 450;

    SetImmersiveMode(GetAndroidApp());
    ANativeActivity_setWindowFlags(GetAndroidApp()->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);  //AWINDOW_FLAG_SCALED, AWINDOW_FLAG_DITHER
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}