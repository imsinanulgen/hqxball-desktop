#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/views/cef_window_delegate.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_keyboard_handler.h"
#include "include/cef_load_handler.h"
#include "include/cef_image.h"
#include <fstream>
#include <vector>
#include <stdlib.h>

#if defined(__APPLE__)
#include "include/wrapper/cef_library_loader.h"
#endif

// Helper function to load a PNG image as a CefImage
CefRefPtr<CefImage> LoadIcon(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return nullptr;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        CefRefPtr<CefImage> image = CefImage::CreateImage();
        image->AddPNG(1.0f, buffer.data(), size);
        return image;
    }
    return nullptr;
}

// Simple client handler
class SimpleClient : public CefClient,
                     public CefLifeSpanHandler,
                     public CefContextMenuHandler,
                     public CefKeyboardHandler,
                     public CefLoadHandler {
public:
  SimpleClient() {}

  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
    return this;
  }

  virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override {
    return this;
  }

  virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override {
    return this;
  }

  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override {
    return this;
  }

  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int httpStatusCode) override {
    if (frame->IsMain()) {
      std::string js = R"(
        (function() {
          function createUI() {
            if (document.getElementById('hqxball-url-bar')) return;
            
            const container = document.createElement('div');
            container.id = 'hqxball-url-bar';
            container.style.cssText = 'position:fixed;top:-100px;left:50%;transform:translateX(-50%);z-index:999999;display:flex;align-items:center;background:rgba(20,20,20,0.8);backdrop-filter:blur(10px);padding:8px 15px;border-radius:20px;box-shadow:0 4px 15px rgba(0,0,0,0.3);transition:all 0.3s ease;opacity:0;';
            
            const input = document.createElement('input');
            input.type = 'text';
            input.placeholder = 'Gitmek istediğiniz odanın linki...';
            input.style.cssText = 'background:transparent;border:none;color:white;width:250px;font-family:sans-serif;font-size:14px;outline:none;';
            
            const btn = document.createElement('button');
            btn.innerText = 'Git';
            btn.style.cssText = 'background:#00d2ff;color:black;border:none;padding:5px 15px;border-radius:15px;cursor:pointer;font-weight:bold;margin-left:10px;';
            
            const toggleBtn = document.createElement('button');
            toggleBtn.id = 'hqxball-url-toggle';
            toggleBtn.innerText = '🔗 Oda Linki';
            toggleBtn.style.cssText = 'position:fixed;top:10px;left:10px;z-index:999999;background:rgba(20,20,20,0.5);color:white;border:1px solid #333;padding:5px 10px;border-radius:10px;cursor:pointer;font-size:12px;font-family:sans-serif;transition:0.2s;';

            let isVisible = false;

            function toggle() {
              isVisible = !isVisible;
              container.style.top = isVisible ? '15px' : '-100px';
              container.style.opacity = isVisible ? '1' : '0';
              if(isVisible) setTimeout(() => input.focus(), 100);
            }

            function go() {
              let url = input.value.trim();
              if (!url) return;
              
              if (url.includes('hqxball.com/play?c=')) {
                if (!url.startsWith('http')) {
                  url = 'https://' + url;
                }
                window.location.href = url;
              } else {
                alert('Lütfen sadece Hqxball oda linklerini giriniz!\nÖrnek: https://www.hqxball.com/play?c=XptCDogQ');
                input.value = '';
                input.focus();
              }
            }

            btn.onclick = go;
            toggleBtn.onclick = toggle;
            toggleBtn.onmouseover = () => toggleBtn.style.background = 'rgba(20,20,20,0.8)';
            toggleBtn.onmouseout = () => toggleBtn.style.background = 'rgba(20,20,20,0.5)';
            
            input.onkeydown = (e) => {
              if (e.key === 'Enter') go();
              if (e.key === 'Escape') toggle();
            };

            container.appendChild(input);
            container.appendChild(btn);
            
            // Wait for body to exist
            const appendUI = () => {
              if (document.body) {
                if (!document.getElementById('hqxball-url-bar')) document.body.appendChild(container);
                if (!document.getElementById('hqxball-url-toggle')) document.body.appendChild(toggleBtn);
              } else {
                setTimeout(appendUI, 100);
              }
            };
            appendUI();
          }
          
          createUI();
          // Continuously check if the UI was removed by SPA navigation and recreate it
          setInterval(() => {
            if (document.body && (!document.getElementById('hqxball-url-bar') || !document.getElementById('hqxball-url-toggle'))) {
              createUI();
            }
          }, 1000);
        })();
      )";
      frame->ExecuteJavaScript(js, frame->GetURL(), 0);
    }
  }

  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
    CefQuitMessageLoop();
  }

  virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefContextMenuParams> params,
                                   CefRefPtr<CefMenuModel> model) override {
    model->Clear();
  }

  virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                             const CefKeyEvent &event, CefEventHandle os_event,
                             bool *is_keyboard_shortcut) override {
    if (event.type == KEYEVENT_RAWKEYDOWN || event.type == KEYEVENT_KEYDOWN) {
      if (event.windows_key_code == 123) { // F12
        return true;
      }
      if ((event.modifiers & EVENTFLAG_CONTROL_DOWN) &&
          (event.modifiers & EVENTFLAG_SHIFT_DOWN)) {
        if (event.windows_key_code == 73 || // I
            event.windows_key_code == 74) { // J
          return true;
        }
      }
    }
    return false;
  }

  IMPLEMENT_REFCOUNTING(SimpleClient);
};

// Window delegate to manage the CEF views window
class SimpleWindowDelegate : public CefWindowDelegate {
public:
  explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    window->SetTitle("Hqxball Desktop Client");

    // Load and set the application icon
    CefRefPtr<CefImage> icon = LoadIcon("assets/icon.png");
    if (icon) {
      window->SetWindowIcon(icon);
      window->SetWindowAppIcon(icon);
    }

    window->SetSize(CefSize(1280, 720));
    window->CenterWindow(CefSize(1280, 720));
    window->AddChildView(browser_view_);
    window->Show();
    browser_view_->RequestFocus();
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) override {
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser) {
      browser->GetHost()->TryCloseBrowser();
    }
    return true;
  }

private:
  CefRefPtr<CefBrowserView> browser_view_;
  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
};

// The main application handler
class SimpleApp : public CefApp, public CefBrowserProcessHandler {
public:
  SimpleApp() {}

  virtual CefRefPtr<CefBrowserProcessHandler>
  GetBrowserProcessHandler() override {
    return this;
  }

  virtual void OnContextInitialized() override {
    CefBrowserSettings browser_settings;
    browser_settings.webgl = STATE_ENABLED;

    CefRefPtr<SimpleClient> client(new SimpleClient());

    // Create the browser view and window
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        client, "https://www.hqxball.com/", browser_settings, nullptr, nullptr,
        nullptr);

    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));
  }

  virtual void OnBeforeCommandLineProcessing(
      const CefString &process_type,
      CefRefPtr<CefCommandLine> command_line) override {
    // High Performance Flags
    command_line->AppendSwitch("disable-frame-rate-limit");
    command_line->AppendSwitch("enable-gpu-rasterization");
    command_line->AppendSwitch("enable-zero-copy");
    command_line->AppendSwitch("ignore-gpu-blocklist");
    command_line->AppendSwitch("disable-software-rasterizer");
    command_line->AppendSwitch("enable-hardware-overlays");

    // Haxball relies on WebRTC, so we do not disable it.
  }

  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#if defined(_WIN32)
#include <windows.h>
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
  CefMainArgs main_args(hInstance);
#else
int main(int argc, char *argv[]) {
  CefMainArgs main_args(argc, argv);
#endif

#if defined(__APPLE__)
  CefScopedLibraryLoader library_loader;
  if (!library_loader.LoadInMain()) {
    return 1;
  }
#endif

  CefRefPtr<SimpleApp> app(new SimpleApp);

  int exit_code = CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0) {
    return exit_code;
  }

  CefSettings settings;
  settings.no_sandbox = true; // Required for simple Linux/Windows setups

#if defined(__APPLE__)
  // Helper App olmadan tek bir executable üzerinden render etmesini sağlıyoruz (Mac App Mimarisi Çökme Çözümü)
  CefString(&settings.browser_subprocess_path).FromASCII(argv[0]);
#endif

#if !defined(_WIN32) && !defined(__APPLE__)
  // Explicitly set paths to avoid ICU and cache errors on Linux
  char abs_path[4096];
  if (realpath(".", abs_path) != nullptr) {
      std::string base_dir(abs_path);
      CefString(&settings.resources_dir_path).FromASCII(base_dir.c_str());
      CefString(&settings.locales_dir_path).FromASCII((base_dir + "/locales").c_str());
      CefString(&settings.root_cache_path).FromASCII((base_dir + "/cache").c_str());
  }
  settings.multi_threaded_message_loop = false;
#elif defined(_WIN32)
  settings.multi_threaded_message_loop = true;
#else
  settings.multi_threaded_message_loop = false;
#endif

  // Initialize CEF.
  CefInitialize(main_args, settings, app, nullptr);

  // Run the CEF message loop.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}
