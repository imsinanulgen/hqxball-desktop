#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/views/cef_window_delegate.h"
#include "include/cef_context_menu_handler.h"
#include "include/cef_keyboard_handler.h"
#include "include/cef_image.h"
#include <fstream>
#include <vector>

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
                     public CefKeyboardHandler {
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
    window->SetTitle("Hqxball Desktop Client - Maximum Performance");

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

    // Disable Safety Tip / Lookalike URL warning
    command_line->AppendSwitchWithValue("disable-features", "LookalikeUrlNavigationSuggestionsUI");
    
    // Disable DevTools
    command_line->AppendSwitch("disable-dev-tools");

    // Haxball relies on WebRTC, so we do not disable it.
  }

  IMPLEMENT_REFCOUNTING(SimpleApp);
};

int main(int argc, char *argv[]) {
  CefMainArgs main_args(argc, argv);

  CefRefPtr<SimpleApp> app(new SimpleApp);

  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  int exit_code = CefExecuteProcess(main_args, app, nullptr);
  if (exit_code >= 0) {
    return exit_code;
  }

  CefSettings settings;
  settings.no_sandbox = true; // Required for simple Linux setups

  // Enable multi-threaded message loop for better performance if supported,
  // but on Linux standard message loop is typically required for Views.
  settings.multi_threaded_message_loop = false;

  // Initialize CEF.
  CefInitialize(main_args, settings, app, nullptr);

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}
