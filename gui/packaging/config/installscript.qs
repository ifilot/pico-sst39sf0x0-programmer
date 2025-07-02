function Component()
{
    component.forceInstallation = true;
}

Component.prototype.createOperations = function()
{
    // Remove QtIFW internal installation marker
    var installInfoPath = installer.value("TargetDir") + "/InstallationInformation";
    var componentsXmlPath = installer.value("TargetDir") + "/components.xml";

    if (QFile.exists(installInfoPath)) {
        installer.performOperation("Delete", installInfoPath);
    }
    if (QFile.exists(componentsXmlPath)) {
        installer.performOperation("Delete", componentsXmlPath);
    }

    component.createOperations();  // must come after cleanup

    // Your existing logic...
    var exePath = installer.value("TargetDir") + "/{{ executable }}.exe";
    var shortcutPath = installer.value("StartMenuDir") + "/{{ executable }}.lnk";

    if (installer.fileExists(shortcutPath)) {
        installer.performOperation("Delete", shortcutPath);
    }

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", 
            "@TargetDir@/{{ executable }}.exe", 
            "@StartMenuDir@/{{ executable }}.lnk",
            "workingDirectory=@TargetDir@", 
            "iconPath=@TargetDir@/{{ iconfilename }}",
            "description=Open {{ progname }}"
        );
    }
}
