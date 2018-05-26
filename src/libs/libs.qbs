import qbs

Project {

	name: "Libs"

	references: [
		"3rdparty/nzmqt/nzmqt.qbs",
		"3rdparty/backward-cpp/backward-cpp.qbs",
        // "zmq-utils/zmq-utils.qbs",
		"core/core.qbs",
	]

}
