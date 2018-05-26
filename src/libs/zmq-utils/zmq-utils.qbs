import qbs

XCppProduct {

	type: "staticlibrary"
	name: "zmq-utils"
	files: [
        "returningdealer.cpp",
        "returningdealer.h",
        "zmqutils.cpp",
        "zmqutils.h",
    ]

	Depends { name: "Qt.core" }
	Depends { name: "Qt.quick" }
	Depends { name: "nzmqt" }

	// Additional import path used to resolve QML modules in Qt Creator's code model
	property pathList qmlImportPaths: []

	cpp.cxxLanguageVersion: "c++11"
	cpp.defines: [
	]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [
            "."
        ]
    }
}
