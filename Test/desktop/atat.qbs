import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: false

    Depends { name:"cpp" }
    cpp.debugInformation: true

    cpp.cFlags: [
        "-std=gnu99"
    ]

    cpp.includePaths: [
            "../../Source/",
        ]

    Group {
            name: "Application"
            prefix: "./"
            files: [
                "*.c",
                "*.cpp",
                "*.h",
            ]
    }
    Group {
        name: "AT-AT"
        prefix: "../../Source/"
        files: [
            "*.c",
            "*.cpp",
            "*.h",
        ]
    }
}

