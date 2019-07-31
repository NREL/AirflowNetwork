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
import tkinter as tk
from tkinter.filedialog import askopenfilename
from tkinter.filedialog import askdirectory
from tkinter.messagebox import showerror
import os

def make_path_input(master, row, text, browse, file_combo=False):
    master.columnconfigure(1,weight=1)
    tk.Label(master, text=text, anchor='e').grid(row=row, sticky='ew')

    var = tk.StringVar()
    if file_combo:
        # Add file/path combobox here
        typevar = tk.BooleanVar()
        tk.Entry(master, textvariable=var).grid(row=row, column=1, sticky='ew')

        tk.Button(master, text="Browse", command=browse).grid(row=row, column=2)
        return var, typevar
    tk.Entry(master, textvariable=var).grid(row=row, column=1, sticky='ew')

    tk.Button(master, text="Browse",
              command=browse).grid(row=row, column=2)
    return var

class ExeFrame(tk.LabelFrame):
    def __init__(self, master, padding=5, width=100):
        super().__init__(master, text="Executable Settings",
                         padx=padding, pady=padding)
        self.exe = make_path_input(self, 0, 'Executable:', self.exe_browse)
        self.wd = make_path_input(self, 1, 'Working Directory:',
                                  self.dir_browse)
        self.columnconfigure(1,weight=1)
    def working_directory(self):
        directory = self.wd.get()
        if os.path.exists(directory) and os.path.isdir(directory):
            return directory
        return None
    def executable(self):
        path = self.exe.get()
        if os.path.exists(path) and os.path.isfile(path):
            return path
        return None
    def exe_browse(self):
        exe_name = askopenfilename(title='Select executable',
                                   filetypes=(('executable files', '*.exe'),
                                              ('all files','*.*')))
        if exe_name:
            self.exe.set(exe_name)
    def dir_browse(self):
        dir_name = askdirectory()
        if dir_name:
            self.wd.set(dir_name)

class Argument:
    def __init__(self, master, row=1):
        self.on = tk.IntVar()
        self.checkbox = tk.Checkbutton(master, variable=self.on,
                                       onvalue=1, offvalue=0)
        self.checkbox.select()
        self.checkbox.grid(row=row, column=0)
        self.option_string = tk.StringVar()
        tk.Entry(master,
                 textvariable=self.option_string).grid(row=row, column=1)
        self.value = tk.StringVar()
        tk.Entry(master,
                 textvariable=self.value).grid(row=row,
                                               column=2,
                                               sticky='ew')
        self.browse_button = tk.Button(master,
                                       text='Browse',
                                       command=self.browse).grid(row=row,
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
    def browse(self):
        result = askopenfilename(title='Select file',
                                   filetypes=(('all files','*.*'),))
        if result:
            self.value.set(result)

class ExpandingFrame(tk.LabelFrame):
    def __init__(self, master, itemclass, padding=5, expandingcol=None,
                 text=None):
        super().__init__(master, text=text,
                         padx=padding, pady=padding)
        if expandingcol:
            self.columnconfigure(expandingcol, weight=1)
        i=self.offset()
        for name in self.labels():
            if i == expandingcol:
                tk.Label(self, text=name).grid(row=0, column=i, sticky='ew')
            else:
                tk.Label(self, text=name).grid(row=0, column=i)
            i += 1
        tk.Button(self, text='Add',
                  command=self.add).grid(row=0, column=i, sticky='ew')
        item = itemclass(self)
        self.itemclass = itemclass
        self.items = [item]
    def offset(self):
        return 0
    def labels(self):
        return []
    def add(self):
        row = len(self.items)+1
        self.items.append(self.itemclass(self,row=row))

class ArgFrame(ExpandingFrame):
    def __init__(self, master, text=None, padding=5, width=100):
        super().__init__(master, Argument, text="Arguments",
                         padding=padding, expandingcol=2)
    def labels(self):
        return ('On/Off', 'Option', 'Value')
    def arguments(self):
        out = []
        for arg in self.items:
            if arg.active():
                out.append(str(arg))
        return out

class Runner(tk.Frame):
    def __init__(self, master, run_button=True, run_callback=None):
        super().__init__(master)

        self.columnconfigure(0, weight=1)

        self.exebox = ExeFrame(self, padding=10) #self.clear_directory, padding=10)
        self.exebox.grid(row=0, sticky='ew')

        self.argbox = ArgFrame(self, padding=10)
        self.argbox.grid(row=1, sticky='ew')

        if run_button:
            self.run_button = tk.Button(self, text='Run', command=self.run)
            self.run_button.grid(row=2, sticky='ew')

        self.callback = None
        if run_callback:
            self.callback = run_callback
    def clear_directory(self):
        path = self.exebox.working_directory()
        if path:
            pass
        else:
            showerror('Clear directory', 'The specified directory does not exist')
    def run(self):
        exe = self.exebox.executable()
        if not exe:
            showerror('Run', 'The specified executable file does not exist')
            return
        path = self.exebox.working_directory()
        if not path:
            showerror('Clear directory', 'The specified directory does not exist')
            return
        options = self.argbox.arguments()
        command = exe + ' ' + ' '.join(options)
        print(command)
        os.chdir(path)
        os.system(command)
        if self.callback:
            self.callback()

if __name__ == '__main__':
    root = tk.Tk()

    root.columnconfigure(0, weight=1)

    app = Runner(root)
    app.grid(sticky='nsew')

    root.mainloop()
    try:
        root.destroy()
    except:
        pass
