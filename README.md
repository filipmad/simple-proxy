# Simple Proxy – COMP30023 Project

A simple HTTP proxy server built as part of **COMP30023: Computer Systems** at the **University of Melbourne** in **2025**.  
Created by **F. Madyarov** and **J. Lewis**.

---

## 🧠 About

This project implements a basic proxy server capable of handling HTTP requests and forwarding them to their intended destinations. The server parses requests, caches responses, and demonstrates a practical understanding of networking, sockets, and system-level programming in C. The proxy implements simple and valid caching with a cache size of 10, and also handles stale cache entries if requested.

---

## 🚀 Features

- Handles multiple client connections using `select()`
- Parses and forwards HTTP GET requests
- Simple in-memory caching for performance improvement
- Handles basic error cases (e.g., unsupported methods, bad requests)
- Command-line interface to run the proxy on a specific port
- Handles `Control Header` caching with internal validation in the cache
- Handles max-age and stale cache entries by evicting expired content and refreshing as needed
  
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

2. **Compile the program**
   ```bash
   make

3. **Start the proxy**
   ```bash
   ./htproxy -p <PORT NUMBER> [optional] -c
   ```
   Bind the proxy to a local port number ```<PORT NUMBER>``` and optionally enable caching by adding ```'-c'``` at the end

4. **Query the proxy**
   ```bash
   curl -x localhost:<PORT NUMBER> http://example.com

## 📚 Coursework Context
This project was completed as part of the University of Melbourne's COMP30023: Computer Systems subject in Semester 1, 2025.
It was developed collaboratively by F. Madyarov and J. Lewis, showcasing concepts such as socket programming, system calls, and concurrency in a real-world scenario.

## ⚖️ License
This code is intended for educational purposes and is subject to the University of Melbourne’s academic integrity policies. Do not plagiarize.

