'Antonin : Paul : thoses may be needed for the process object
'import for controlling processes
Imports System
Imports System.Diagnostics
Imports System.ComponentModel



Public Class Form1


    'this variable is supposed to contain the output of the programm
    ' we'll have to test if not empty when modified and if not empty, we parse it !
    Dim ConsoleProgramOutputLine As String

    'this variable is here to test the parse function
    Dim TrialOutputToParse As String = "success: Xshift = 1.11; Yshift = 2.22;"

    'to store x and y shift
    Dim xtr, ytr As Double





    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button1.Click
        ConsoleProgramOutputLine = StartProcess("cmd.exe", "/C dir", "C:\Users\Antonin\")

    End Sub

    Public Function StartProcess(ByVal processName As String, ByVal processParam As String, ByVal workingDir As String) As String
        Dim returnLine As String = ""
        Me.TextBox1.Clear()
        Dim clsProcess As New System.Diagnostics.Process()
        clsProcess.StartInfo.UseShellExecute = False
        clsProcess.StartInfo.RedirectStandardOutput = True
        clsProcess.StartInfo.RedirectStandardError = True
        clsProcess.StartInfo.FileName = processName
        clsProcess.StartInfo.Arguments = processParam
        clsProcess.StartInfo.WorkingDirectory = workingDir
        clsProcess.StartInfo.CreateNoWindow = True
        clsProcess.Start()

        While (clsProcess.HasExited = False)
            Dim sLine As String = clsProcess.StandardOutput.ReadLine
            If (Not String.IsNullOrEmpty(sLine)) Then
                Application.DoEvents()
                Me.TextBox1.ScrollToCaret() ' unfortunately carret seems not to be in the end of the textbox
                'append program current output to current textbox contents
                Me.TextBox1.Text += (sLine & vbCrLf)
                'here is the important keyword for the output line of the program
                If sLine.Contains("success:") Then
                    returnLine = sLine
                End If
            End If
        End While
        Me.TextBox1.ScrollToCaret()
        Me.TextBox1.Text += "[Completed]"
        clsProcess.Close()
        Return returnLine
    End Function






End Class
