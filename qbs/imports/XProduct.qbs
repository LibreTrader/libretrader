import qbs

Product {
	
    property string platform: {

        // TODO: when qbs.architecture bug is fixed,
        // use it to do os/arch split.
        // Right now there is only OS split.
        // Other option is to code arch detection here...

        if (qbs.targetOS.contains("linux"))
                return "linux"
        if (qbs.targetOS.contains("windows"))
                return "windows"
        if (qbs.targetOS.contains("macos"))
                return "macos"
        return "unknown"

    }

}
