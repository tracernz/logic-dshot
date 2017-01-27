import qbs
import qbs.Environment

Project {
    name: "logic-dshot"

    property string revision: {
        var rev = "dev";
        if (Environment.getEnv("TRAVIS")) {
            rev = Environment.getEnv("TRAVIS_TAG");
            if (!rev) {
                rev = Environment.getEnv("TRAVIS_COMMIT");
                if (rev)
                    rev = rev.substring(0, 7);
            }
        } else if (Environment.getEnv("APPVEYOR")) {
            if (Environment.getEnv("APPVEYOR_REPO_TAG") === "true") {
                rev = Environment.getEnv("APPVEYOR_REPO_TAG_NAME");
            } else {
                rev = Environment.getEnv("APPVEYOR_REPO_COMMIT");
                if (rev)
                    rev = rev.substring(0, 7);
            }
        }
        return rev ? rev : "dev";
    }

    property string targetOS: qbs.targetOS.contains("linux") ? "linux" : qbs.targetOS.contains("macos") ? "macos" : qbs.targetOS.contains("windows") ? "win" : qbs.targetOS[0]

    DynamicLibrary {
        name: "DshotAnalyzer"
        targetName: qbs.buildVariant !== "release" ? [name, qbs.buildVariant].join("_") : name

        Depends { name: "cpp" }

        cpp.debugInformation: qbs.buildVariant == "debug" ? true : false
        cpp.optimization: qbs.buildVariant == "debug" ? "none" : "fast"
        cpp.includePaths: ["sdk/include", "source"]
        cpp.libraryPaths: ["sdk/lib"]
        cpp.cxxLanguageVersion: "c++11"
        cpp.minimumMacosVersion: "10.9"

        Properties {
            condition: cpp.architecture.indexOf("64") != -1 && !qbs.targetOS.contains("macos")
            cpp.dynamicLibraries: ["Analyzer64"]
        }
        cpp.dynamicLibraries: ["Analyzer"]

        Group {
            name: "headers"
            files: "source/*.h"
        }

        Group {
            name: "sources"
            files: "source/*.cpp"
        }

        Group {
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: "."
        }
    }

    InstallPackage {
        name: "package"
        archiver.type: qbs.targetOS.contains("windows") ? "zip" : "tar"
        archiver.compressionType: archiver.type === "tar" ? "xz" : undefined

        Depends {
            productTypes: [
                "application",
                "dynamiclibrary",
                "installable",
            ]
        }

        Depends { name: "cpp" }

        targetName: [project.name, project.targetOS, cpp.architecture, project.revision].join("-")

        qbs.install: true
        qbs.installDir: "."
    }
}
