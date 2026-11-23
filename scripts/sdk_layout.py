"""Source package layout shared by the SDK generators."""


def java_wrapper(text, area, profile="server"):
    root = "org.basaltmc." + profile
    text = text.replace("package basalt;", "package " + root + "." + area + ";\nimport " + root + ".internal.*;\nimport " + root + ".core.*;")
    text = text.replace("import basalt.Basalt.Args;", "").replace("import basalt.Basalt.Handler;", "")
    text = text.replace("basalt.Values", root + ".values.Documents")
    return text.replace("Basalt.tokenFor", "Callbacks.tokenFor").replace("Basalt.Handler", "Handler").replace("Basalt.Args", "Args").replace("Basalt.", "Native.").replace("basalt.Native", root + ".internal.Native").replace("basalt.Raw", root + ".internal.Raw")
