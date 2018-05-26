var components = Object()

function createComponent(type) {
    if (!(type in components)) {
        var qmlfile = type + ".qml"
        components[type] = Qt.createComponent(qmlfile)
    }
}

function getComponentSync(type) {
    createComponent(type)
    var component = components[type]
    while (component.status === Component.Loading) { }
    if (component.status === Component.Error) {
        console.error("error loading component '" + type + "': " +
                      component.errorString())
    }
    return components[type]
}

function createObjSync(type, parent, args) {
    var component = getComponentSync(type)
    args = args == null ? {} : args
    return component.createObject(parent, args)
}

function Timer() {
    return Qt.createQmlObject("import QtQuick 2.0; Timer {}", root);
}

function delayed(delay, callback) {
    var timer = new Timer()
    timer.interval = delay
    timer.repeat = false
    timer.triggered.connect(callback)
    timer.start()
}
