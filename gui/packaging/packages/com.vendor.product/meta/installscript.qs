function Component()
{
    // default constructor
    component.forceInstallation = true;
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    // Remove old executable if it exists (optional cleanup)
    var oldExe = "@TargetDir@/pico-sst39sf0x0-programmer.exe"
    if (QFile.exists(oldExe)) {
        installer.performOperation("Delete", oldExe);
    }

    // Remove old shortcut if it exists (optional cleanup)
    var oldShortcut = "@StartMenuDir@/pico-sst39sf0x0-programmer.lnk";
    if (QFile.exists(oldShortcut)) {
        installer.performOperation("Delete", oldShortcut);
    }

    // Windows-specific shortcut creation
    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/pico-sst39sf0x0-programmer.exe", 
                               "@StartMenuDir@/pico-sst39sf0x0-programmer.lnk",
                               "workingDirectory=@TargetDir@", 
                               "iconPath=@TargetDir@/eeprom_icon.ico",
                               "description=Open PICO SST39SF0x0 Programmer");
    }

    // Optional: Run the application after install
    component.addOperation("Execute", 
        "@TargetDir@/pico-sst39sf0x0-programmer.exe", 
        "", 
        "UNDOEXECUTE", 
        "@TargetDir@/pico-sst39sf0x0-programmer.exe"
    );
}