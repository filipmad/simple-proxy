# UniMelb Proxy – COMP30023 Project

A simple HTTP proxy server built as part of **COMP30023: Computer Systems** at the **University of Melbourne** in **2025**.  
Created by **[Your Name]** and **J. Lewis**.

---

## 🧠 About

This project implements a basic proxy server capable of handling HTTP requests and forwarding them to their intended destinations. The server parses requests, caches responses, and demonstrates a practical understanding of networking, sockets, and system-level programming in C.

---

## 🚀 Features

- Handles multiple client connections using `select()`
- Parses and forwards HTTP GET requests
- Simple in-memory caching for performance improvement
- Handles basic error cases (e.g., unsupported methods, bad requests)
- Command-line interface to run the proxy on a specific port

---

## 🛠️ Technologies Used

- C (POSIX sockets)
- Standard C libraries (`stdio`, `stdlib`, `string`, etc.)
- Linux/MacOS development environment

---

## 🧪 How to Run

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/unimelb-proxy.git
   cd unimelb-proxy



## 📚 Coursework Context
This project was completed as part of the University of Melbourne's COMP30023: Computer Systems subject in Semester 1, 2025.
It was developed collaboratively by [Your Name] and J. Lewis, showcasing concepts such as socket programming, system calls, and concurrency in a real-world scenario.

