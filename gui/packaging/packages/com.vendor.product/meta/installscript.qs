function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/pico-sst39sf0x0-programmer.exe", 
                               "@StartMenuDir@/pico-sst39sf0x0-programmer.lnk",
                               "workingDirectory=@TargetDir@", 
                               "iconPath=@TargetDir@/eeprom_icon.ico",
                               "description=Open PICO SST39SF0x0 Programmer");
    }
}