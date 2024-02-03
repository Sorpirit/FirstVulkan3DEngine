using System;
using System.Collections.Generic;
using System.IO;

string shadersFolder = Args[0];
string outputFolder = Args[1];
string buildTool = Args[2];
string outputFormat = ".spv";


public void Compile(string shaderPath){
    string shaderName = Path.GetFileName(shaderPath) + outputFormat;
    string arguments = $"{shaderPath} -o {Path.Combine(outputFolder, shaderName)}";
    Console.WriteLine(arguments);
    Process p = Process.Start(new ProcessStartInfo()
    {    
        FileName = buildTool,
        Arguments = arguments,
        UseShellExecute = false
    });
    p.WaitForExit();
}

foreach(var shader in Directory.GetFiles(shadersFolder, "*", SearchOption.AllDirectories)){
    Compile(Path.GetFullPath(shader));
}

