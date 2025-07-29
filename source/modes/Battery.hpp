class BatteryOverlay : public tsl::Gui {
private:
    char Battery_c[512];
public:
    BatteryOverlay() {
        disableJumpTo = true;
        mutexInit(&mutex_BatteryChecker);
        StartBatteryThread();
    }
    ~BatteryOverlay() {
        CloseThreads();
    }

    virtual tsl::elm::Element* createUI() override {
        tsl::elm::OverlayFrame* rootFrame = new tsl::elm::OverlayFrame("Status Monitor 性能监控", "葡萄糖酸菜鱼 汉化", true);

        auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawString("电池与充电数据:", false, 20, 120, 20, 0xFFFF);
            renderer->drawString(Battery_c, false, 20, 155, 18, 0xFFFF);
        });

        rootFrame->setContent(Status);

        return rootFrame;
    }

    virtual void update() override {

        ///Battery

        mutexLock(&mutex_BatteryChecker);
        char tempBatTimeEstimate[8] = "--:--";
        if (batTimeEstimate >= 0) {
            snprintf(&tempBatTimeEstimate[0], sizeof(tempBatTimeEstimate), "%d:%02d", batTimeEstimate / 60, batTimeEstimate % 60);
        }

        BatteryChargeInfoFieldsChargerType ChargerConnected = _batteryChargeInfoFields.ChargerType;
        int32_t ChargerVoltageLimit = _batteryChargeInfoFields.ChargerVoltageLimit;
        int32_t ChargerCurrentLimit = _batteryChargeInfoFields.ChargerCurrentLimit;

        if (hosversionAtLeast(17,0,0)) {
            ChargerConnected = ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields) -> ChargerType;
            ChargerVoltageLimit = ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields) -> ChargerVoltageLimit;
            ChargerCurrentLimit = ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields) -> ChargerCurrentLimit;
        }

        if (ChargerConnected)
            snprintf(Battery_c, sizeof Battery_c,
                "电池实际容量: %.0f mAh\n"
                "电池设计容量: %.0f mAh\n"
                "电池温度: %.1f℃\n"
                "真实电量: %.1f%%\n"
                "电池寿命: %.1f%%\n"
                "电压(%d秒平均值): %.0f mV\n"
                "电流(%s秒平均值): %+.0f mA\n"
                "电池功率%s: %+.3f W\n"
                "剩余充电时间: %s\n"
                "输入电流限制: %d mA\n"
                "VBUS电流限制: %d mA\n" 
                "充电模式: %u\n"
                "最大充电电压: %u mV\n"
                "最大充电电流: %u mA",
                actualFullBatCapacity,
                designedFullBatCapacity,
                (float)_batteryChargeInfoFields.BatteryTemperature / 1000,
                (float)_batteryChargeInfoFields.RawBatteryCharge / 1000,
                (float)_batteryChargeInfoFields.BatteryAge / 1000,
                batteryFiltered ? 45 : 5, batVoltageAvg,
                batteryFiltered ? "11.25" : "5", batCurrentAvg,
                batteryFiltered ? "" : "(5秒平均值)", PowerConsumption, 
                tempBatTimeEstimate,
                _batteryChargeInfoFields.InputCurrentLimit,
                _batteryChargeInfoFields.VBUSCurrentLimit,
                ChargerConnected,
                ChargerVoltageLimit,
                ChargerCurrentLimit
            );
        else
            snprintf(Battery_c, sizeof Battery_c,
                "电池实际容量: %.0f mAh\n"
                "电池设计容量: %.0f mAh\n"
                "电池温度: %.1f℃\n"
                "真实电量: %.1f%%\n"
                "电池寿命: %.1f%%\n"
                "电压(%d秒平均值): %.0f mV\n"
                "电流(%s秒平均值): %+.0f mA\n"
                "电池功率%s: %+.3f W\n"
                "电池剩余续航: %s",
                actualFullBatCapacity,
                designedFullBatCapacity,
                (float)_batteryChargeInfoFields.BatteryTemperature / 1000,
                (float)_batteryChargeInfoFields.RawBatteryCharge / 1000,
                (float)_batteryChargeInfoFields.BatteryAge / 1000,
                batteryFiltered ? 45 : 5, batVoltageAvg,
                batteryFiltered ? "11.25" : "5", batCurrentAvg,
                batteryFiltered ? "" : "(5秒平均值)", PowerConsumption, 
                tempBatTimeEstimate
            );
        mutexUnlock(&mutex_BatteryChecker);
        
        static bool skipOnce = true;

        if (!skipOnce) {
            static bool runOnce = true;
            if (runOnce)
                isRendering = true;
        } else {
            skipOnce = false;
        }
    }
    virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (keysDown & KEY_B) {
            isRendering = false;
            tsl::goBack();
            return true;
        }
        return false;
    }
};
