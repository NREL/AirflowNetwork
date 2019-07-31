# Copyright (c) 2019, Alliance for Sustainable Energy, LLC
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
import runner
import tkinter as tk
from tkinter.filedialog import askopenfilename
from tkinter.filedialog import askdirectory
#from tkinter.messagebox import showerror

class DirectoryInput(tk.Frame):
    def __init__(self, master, text=None, padding=5):
        super().__init__(master, padx=padding, pady=padding)
        if not text:
            text = 'Directory'
        self.path = runner.make_path_input(self, 0, text, self.browse)
    def directory(self):
        path = self.path.get()
        if os.path.exists(path) and os.path.isdir(path):
            return path
        return None
    def browse(self):
        dir_name = askdirectory()
        if dir_name:
            self.dir.set(dir_name)

class FileDiff:
    def __init__(self, master, row=1):
        self.on = tk.IntVar()
        self.checkbox = tk.Checkbutton(master, variable=self.on,
                                       onvalue=1, offvalue=0)
        self.checkbox.select()
        self.checkbox.grid(row=row, column=0)
        self.diff = tk.StringVar()
        tk.Entry(master,
                 textvariable=self.diff).grid(row=row, column=1)
        tk.Button(self, text="Browse",
                  command=self.exe_browse).grid(row=row, column=2)
        
        self.file = tk.StringVar()
        tk.Entry(master,
                 textvariable=self.file).grid(row=row,
                                              column=3,
                                              sticky='ew')
        self.browse_button = tk.Button(master,
                                       text='Browse',
                                       command=self.file_browse).grid(row=row,
                                                                 column=3)
    def __str__(self):
        prefix = self.option_string.get()
        if prefix and not prefix.isspace():
            prefix += ' '
        else:
            prefix = ''
        # Should probably do some work here on spaces, etc.
        return prefix + self.value.get()
    def active(self):
        return self.on.get() == 1
    def executable(self):
        path = self.diff.get()
        if os.path.exists(path) and os.path.isfile(path):
            return path
        return None
    def exe_browse(self):
        exe_name = askopenfilename(title='Select diff executable',
                                   filetypes=(('executable files', '*.exe'),
                                              ('all files','*.*')))
        if exe_name:
            self.exe.set(exe_name)
    def file_browse(self):
        result = askopenfilename(title='Select file',
                                   filetypes=(('all files','*.*'),))
        if result:
            result = os.path.basename(result)
            self.value.set(result)

class FileDiffs(runner.ExpandingFrame):
    def __init__(self, master, text=None, padding=5, width=100):
        super().__init__(master, FileDiff, text="File Diffs",
                         padding=padding)
        self.project_dir = DirectoryInput(root, text='Project Directory:',
                                          padding=10)
        self.project_dir.grid(row=0, columnspan=4, sticky='ew')
    def offset(self):
        return 0
    def labels(self):
        return ('On/Off', 'Diff Executable', '', 'File', '')
    def arguments(self):
        out = []
        for arg in self.items:
            if arg.active():
                out.append(str(arg))
        return out

class DiffRunner(tk.Frame):
    def __init__(self, master):
        super().__init__(master)
        
        self.columnconfigure(0, weight=1)
        self.columnconfigure(1, weight=1)

        self.project_dir = DirectoryInput(root, text='Project Directory:',
                                          padding=10)
        self.project_dir.grid(row=0, columnspan=2, sticky='ew')

        baseframe = tk.LabelFrame(root, text='Baseline')
        baseframe.columnconfigure(0, weight=1)
        baseframe.columnconfigure(1, weight=1)
        baseframe.grid(row=1,column=0,sticky='nsew')

        baseline = runner.Runner(baseframe) #, run_button=False)
        baseline.grid(sticky='nsew')

        diffframe = tk.LabelFrame(root, text='Alternative')
        diffframe.columnconfigure(0, weight=1)
        diffframe.columnconfigure(1, weight=1)
        diffframe.grid(row=1, column=1, sticky='nsew')

        different = runner.Runner(diffframe) #, run_button=False)
        different.grid(row=0, column=1, sticky='nsew')

        filediffs = FileDiffs(self)
        filediffs.grid(row=2, columnspan=2, sticky='ew')

if __name__ == '__main__':
    root = tk.Tk()

    root.columnconfigure(0, weight=1)
    #root.columnconfigure(1, weight=1)

    #project_dir = ProjectDir(root, padding=10)
    #project_dir.grid(row=0, columnspan=2, sticky='ew')

    #baseframe = tk.LabelFrame(root, text='Baseline')
    #baseframe.columnconfigure(0, weight=1)
    #baseframe.columnconfigure(1, weight=1)
    #baseframe.grid(row=1,column=0,sticky='nsew')

    #baseline = runner.Runner(baseframe, run_button=False)
    #baseline.grid(sticky='nsew')

    #diffframe = tk.LabelFrame(root, text='Alternative')
    #diffframe.columnconfigure(0, weight=1)
    #diffframe.columnconfigure(1, weight=1)
    #diffframe.grid(row=1, column=1, sticky='nsew')

    #different = runner.Runner(diffframe, run_button=False)
    #different.grid(row=0, column=1, sticky='nsew')

    app = DiffRunner(root)
    app.grid(sticky='nsew')

    root.mainloop()
    try:
        root.destroy()
    except:
        pass
