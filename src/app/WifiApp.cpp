#include "WifiApp.hpp"
#include "WifiCli.hpp"
#include "SetupHandler.hpp"
#include "utilities/print/Print.hpp"

using namespace Wifi;

const static char* SETUP_AP_NAME = "HOPPER_SETUP";
const static char* SETUP_AP_PASS = NULL;
const static uint16_t PORT = 80;

IPAddress localIp(192,168,4,1);
IPAddress gateway(192,168,4,2);
IPAddress subnet(255,255,255,0);

namespace Hopper
{
    WifiApp::WifiApp(Wifi::WifiController* pWifi, HopperSocket* pSocket):
        pWifi_(pWifi),
        pSocket_(pSocket),
        socketRetryTimer_(RETRY_MS),
        state_(NORMAL)
    {
        pSocket->registerSocket();

        WiFi.setAutoConnect(false);
        WiFi.persistent(false);
    }

    void WifiApp::start()
    {
        loadConfigs();

        if ((strlen(ssid_) > 0) && hopperId_ > 0)
        {
            startSocket();
        }
        else
        {
            enterSetup();
        }
    }

    void WifiApp::loadConfigs()
    {
        eeprom_getWifiCreds(ssid_, password_);
        hopperId_ = eeprom_getId();

    #ifdef DEBUG
        PRINTLN("\n");
        PRINTLN("ID: %d", hopperId_);
        PRINTLN("SSID: %s", ssid_);
        PRINTLN("PASS: %s", password_);
    #endif
    }

    AppState WifiApp::getState()
    {
        return state_;
    }

    void WifiApp::update()
    {
        switch (state_)
        {
            case NORMAL:
                pSocket_->update();

                // Retry connecting the socket periodically
                if (!pSocket_->isConnected())
                {
                    if (!socketRetryTimer_.isRunning())
                    {
                        LOG_LOCAL("RT S");
                        socketRetryTimer_.start();
                    }

                    if (socketRetryTimer_.hasPeriodPassed())
                    {
                        LOG_LOCAL("RT E");
                        startSocket();
                    }
                }
                break;

            case SETUP:
                pWifi_->updateServer(pSetupServer_, &clientHandler);
                break;

            default:
                DEBUG_PRINTLN("App state err.");
                state_ = NORMAL;
                break;
        }
    }

    void WifiApp::startSocket()
    {
        socketRetryTimer_.stop();

        state_ = NORMAL;

        pSocket_->connect(hopperId_, ssid_, password_);
    }

    bool WifiApp::isSocketConnected()
    {
        return (state_ == NORMAL) && pSocket_->isConnected();
    }

    void WifiApp::log(const char* message)
    {
        if ((state_ == NORMAL) && (pSocket_->isConnected()))
        {
            pSocket_->log(message);
        }
    }

    void WifiApp::enterSetup()
    {
        state_ = SETUP;

        if (pSocket_->isConnected())
        {
            pSocket_->disconnect();
            pSocket_->update();
        }

        pWifi_->disconnect();
        pWifi_->startAp(SETUP_AP_NAME, SETUP_AP_PASS, localIp, gateway, subnet);

        pSetupServer_ = pWifi_->startServer(PORT);
    }

    void WifiApp::exitSetup()
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            // Save credentials since we were able to connect with them
            eeprom_setWifiCreds(ssid_, password_);
        }

        if (pSetupServer_ != NULL)
        {
            pSetupServer_->close();
            pSetupServer_ = NULL;
        }

        pWifi_->disconnect();
        pWifi_->shutOff();

        startSocket();
    }

    bool WifiApp::testConnect(const char* ssid, const char* password)
    {
        strncpy(ssid_, ssid, MAX_SSID_LEN);
        strncpy(password_, password, MAX_PASSWORD_LEN);

        WiFi.begin(ssid_, password_);

        return true;
    }
}