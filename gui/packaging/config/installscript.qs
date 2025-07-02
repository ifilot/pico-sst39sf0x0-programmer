function Controller() {}

Controller.prototype.TargetDirectoryPageCallback = function() {
    var page = gui.pageWidgetByObjectName("TargetDirectoryPage");

    // Override the default validation function
    page.targetDirectoryLineEdit().textChanged.connect(function(dir) {
        // Always allow the user to continue
        page.completeChanged();
    });

    page.isComplete = function() {
        var dir = page.targetDirectoryLineEdit().text;

        // If the directory exists and has an existing installation, we still allow it
        return true;
    };
};