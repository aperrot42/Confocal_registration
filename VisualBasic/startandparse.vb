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













    Public Sub parseOutput(ByRef stringToParse, ByRef xTranslation, ByRef yTranslation)

        Dim XshiftIndexStart, XshiftIndexEnd, YshiftIndexStart, YshiftIndexEnd As Integer
        Dim stringXShift, stringYShift As String

        'Let's try to parse the line containing X and Y shifts from registration program
        Try

            ' Find index position of the marker for Yposition and Xposition
            XshiftIndexStart = stringToParse.IndexOf("Xshift = ") + 9
            XshiftIndexEnd = stringToParse.IndexOf(";", XshiftIndexStart)

            YshiftIndexStart = stringToParse.IndexOf("Yshift = ") + 9
            YshiftIndexEnd = stringToParse.IndexOf(";", YshiftIndexStart)

            'We extract the part of the string between the "Xshift = " and ";"
            stringXShift = stringToParse.Substring(XshiftIndexStart, XshiftIndexEnd - (XshiftIndexStart + 1))
            stringYShift = stringToParse.Substring(YshiftIndexStart, YshiftIndexEnd - (YshiftIndexStart + 1))
            Me.xlabel.Text = stringYShift
            'conversion of substrings to double
            xTranslation = Val(stringXShift)
            yTranslation = Val(stringYShift)


        Catch OverflowException As Exception
            Console.WriteLine("Error trying to read output of registration program." + ControlChars.Lf + ControlChars.Lf)
        End Try


    End Sub



    Private Sub Button2_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button2.Click
        labelToParse.Text = TrialOutputToParse
        parseOutput((TrialOutputToParse), xtr, ytr)
        Me.NumericUpDown1.Value = xtr
        Me.NumericUpDown2.Value = ytr
    End Sub
End Class
