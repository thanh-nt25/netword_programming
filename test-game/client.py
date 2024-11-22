import socket
import tkinter as tk
from tkinter import messagebox

HOST = '127.0.0.1'
PORT = 8080

class CrosswordPuzzle:
    def __init__(self, master):
        self.master = master
        self.master.title("Crossword Puzzle")
        self.puzzle = [
            ['C', 'A', 'T', ' ', ' '],
            [' ', ' ', 'A', ' ', ' '],
            ['D', 'O', 'G', ' ', ' '],
            [' ', ' ', ' ', ' ', ' '],
            [' ', 'B', 'I', 'R', 'D']
        ]

        self.create_widgets()
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.client_socket.connect((HOST, PORT))

    def create_widgets(self):
        self.entries = []
        for i in range(5):
            row_entries = []
            for j in range(5):
                text = self.puzzle[i][j] if self.puzzle[i][j] != ' ' else ""
                entry = tk.Entry(self.master, width=2, font=('Arial', 18), justify='center')
                entry.grid(row=i, column=j, padx=5, pady=5)
                entry.insert(0, text)
                entry.config(state='normal' if text == "" else 'disabled')
                row_entries.append(entry)
            self.entries.append(row_entries)

        self.submit_button = tk.Button(self.master, text="Submit", command=self.submit)
        self.submit_button.grid(row=6, column=0, columnspan=5)

    def submit(self):
        for i in range(5):
            for j in range(5):
                if self.entries[i][j]['state'] == 'normal':
                    letter = self.entries[i][j].get().strip().upper()
                    if letter:
                        self.client_socket.send(f"{i} {j} {letter}\n".encode())
                        response = self.client_socket.recv(1024).decode().strip()
                        if response == "CORRECT":
                            self.entries[i][j].config(state='disabled')
                        else:
                            messagebox.showerror("Error", f"Wrong letter at ({i}, {j})")

if __name__ == "__main__":
    root = tk.Tk()
    game = CrosswordPuzzle(root)
    root.mainloop()

