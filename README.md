# Task Management Application - Complete Guide

**Network Programming Final Project - Client-Server TCP Socket Application**

---

## 📋 Table of Contents

1. [Quick Demo Setup](#quick-demo-setup-3-terminals)
2. [What's Implemented](#whats-implemented)
3. [Demo Script](#complete-demo-script)
4. [Client Menu Reference](#client-menu-reference)
5. [How It Works](#how-it-works)
6. [API Reference (For Your UI)](#api-reference-for-your-ui)
7. [Troubleshooting](#troubleshooting)

---

## Quick Demo Setup (3 Terminals)

### ✅ What's Ready:
- Server executable: `build/server` ✅
- Client executable: `build/client` ✅
- Database: PostgreSQL configured ✅
- Hardcoded: `127.0.0.1:8080` (localhost) ✅

### 🚀 Run Your Demo:

**Terminal 1 - Start Server:**
```bash
cd ~/NetworkProgrammingFinalProject/HUST-NetworkProgramming-TaskManagement
./build/server
```

**Expected Output:**
```
===== Task Management Server =====
Database connected successfully
Server listening on port 8080
```

**Terminal 2 - Start Client 1:**
```bash
cd ~/NetworkProgrammingFinalProject/HUST-NetworkProgramming-TaskManagement
./build/client
```

**Terminal 3 - Start Client 2:**
```bash
cd ~/NetworkProgrammingFinalProject/HUST-NetworkProgramming-TaskManagement
./build/client
```

**🎉 You now have 1 server + 2 clients running!**

### ⚠️ Critical Notes:

1. **OTP codes appear in SERVER terminal (Terminal 1)** - Keep it visible!
2. Both clients need the **same PROJECT ID** to collaborate
3. Use menu option **3** to get your project IDs
4. Server **must be running** before starting clients

### 🧪 Test Accounts (Optional):

If you ran `./DB/load_sample_data.sh`:
- **Username:** `john_doe`, **Password:** `password123`
- **Username:** `jane_smith`, **Password:** `password123`

Otherwise, just register new accounts during demo!

---

## What's Implemented

### ✅ Complete Features:

**Server (Multi-threaded TCP):**
- ✅ Handles multiple clients simultaneously
- ✅ Binary protocol with JSON payloads
- ✅ PostgreSQL database (10 tables)
- ✅ Session management
- ✅ Message logging

**Authentication:**
- ✅ User registration
- ✅ Login with credentials
- ✅ OTP verification (printed to server terminal)
- ✅ Password hashing (SHA256)
- ✅ Session tracking

**Project Management:**
- ✅ Create projects
- ✅ Update project details
- ✅ View user's projects
- ✅ Invite members
- ✅ Owner permissions

**Task Management:**
- ✅ Create tasks with deadlines
- ✅ Assign tasks to users
- ✅ Update task status (todo/in_progress/completed)
- ✅ Set priority (low/medium/high/urgent)
- ✅ Delete tasks

**Chat:**
- ✅ Send messages in projects
- ✅ View chat history
- ✅ Real-time messaging

**Contacts:**
- ✅ Add friends
- ✅ View contact list

**Files:**
- ✅ Upload file metadata
- ✅ View task files

### 🔧 Configuration:

All hardcoded for localhost demo:
- **Server IP:** `127.0.0.1`
- **Server Port:** `8080`
- **Database:** `netprog_db`
- **DB User:** `netprog_user`
- **DB Password:** `netprog_password`

---

## Complete Demo Script

### Demo Scenario: Two Users Collaborating on a Project

## 👤 USER 1: John (Terminal 2)

**1. Start Client:**
```
(Already running: ./build/client)
```

**2. Register (if no test accounts):**
```
Choose option: 1
Username: john
Email: john@example.com
Password: password123
```

**Watch Terminal 1 (server) for OTP:**
```
========== EMAIL SERVICE ==========
OTP Code: 123456
===================================
```
Enter the OTP when prompted.

**3. Login:**
```
Choose option: 2
Username: john
Password: password123
```
Check server terminal for OTP and enter it.

**4. Create a Project:**
```
Choose option: 4
Project name: Final Project
Description: Network Programming Final Project
```

**Note the project ID printed!** (e.g., `proj-12345678-abcd-...`)

**5. Create First Task:**
```
Choose option: 6
Task name: Implement Server
Description: Build TCP server with socket programming
Priority: high
Due date: 2024-12-31
```

**6. Send Chat Message:**
```
Choose option: 8
Message: Hey! I just created our project and first task.
```

---

## 👤 USER 2: Jane (Terminal 3)

**1. Start Client:**
```
(Already running: ./build/client)
```

**2. Register:**
```
Choose option: 1
Username: jane
Email: jane@example.com
Password: password123
```
Check server terminal for OTP and enter it.

**3. Login:**
```
Choose option: 2
Username: jane
Password: password123
```
Check server terminal for OTP and enter it.

**4. View Chat (use John's project ID):**
```
Choose option: 9
Enter project ID: proj-12345678-abcd-...  (copy from John's screen)
```

**You'll see John's message!** ✨

**5. Reply in Chat:**
```
Choose option: 8
Message: Hi John! I'll work on the client side.
```

**6. Create Second Task:**
```
Choose option: 6
Task name: Implement Client
Description: Build client with network manager
Priority: high
Due date: 2024-12-31
```

---

## 🔄 Show Real-Time Updates

### Switch to Terminal 2 (John):

**View Updated Chat:**
```
Choose option: 9
```
**You'll see Jane's reply!** ✨

**View All Tasks:**
```
Choose option: 5
```
**You'll see both tasks!** (John's and Jane's)

**Update Task Status:**
```
Choose option: 7
Task ID: task-xxxxx  (from your task)
New status: in_progress
```

---

### Switch to Terminal 3 (Jane):

**View Tasks Again:**
```
Choose option: 5
```
**You'll see John's task is now "in_progress"!** ✨

**Add John as Contact:**
```
Choose option: 10
Username or email: john
```

**Mark Your Task Complete:**
```
Choose option: 7
Task ID: task-xxxxx  (your task)
New status: completed
```

---

### Switch Back to Terminal 2 (John):

**View Projects:**
```
Choose option: 3
```
See your project listed.

**View Tasks Again:**
```
Choose option: 5
```
**Both tasks updated!** John's is in_progress, Jane's is completed! ✨

---

## Client Menu Reference

When you run `./build/client`, you'll see this menu:

```
========================================
  Logged in as: [username] (or "Not logged in")
========================================
1.  Register               - Create new account
2.  Login                  - Login with credentials
3.  Get My Projects        - View all your projects
4.  Create Project         - Create new project
5.  Get Tasks              - View tasks for current project
6.  Create Task            - Create new task
7.  Update Task Status     - Change task status
8.  Send Chat Message      - Send message in project
9.  View Chat History      - View project chat
10. Add Contact            - Add friend
11. View Contacts          - View your contacts
12. Logout                 - Logout
0.  Exit                   - Close client
========================================
Choose option:
```

### Menu Details:

**Option 1 - Register:**
- Enter: username, email, password
- **Watch server terminal for OTP code!**
- Enter OTP when prompted

**Option 2 - Login:**
- Enter: username, password
- **Watch server terminal for OTP code!**
- Enter OTP when prompted
- After successful login, you can use all other features

**Option 3 - Get My Projects:**
- Shows all projects you own or are a member of
- Displays raw JSON response
- **Copy the project_id from output to use in other options**

**Option 4 - Create Project:**
- Enter: project name, description
- **Save the project ID shown after creation!**
- The project ID is automatically set as current project

**Option 5 - Get Tasks:**
- If no current project, asks for project ID
- Shows all tasks in the project
- Displays raw JSON response

**Option 6 - Create Task:**
- Enter: task name, description, priority, due date
- Priority: `low`, `medium`, `high`, or `urgent`
- Due date format: `YYYY-MM-DD` (e.g., `2024-12-31`)
- Task is created in current project

**Option 7 - Update Task Status:**
- Enter: task ID, new status
- Status: `todo`, `in_progress`, `completed`, `in_review`, or `blocked`
- Get task ID from option 5 output

**Option 8 - Send Chat Message:**
- Enter: message text
- Sends to current project's chat
- All project members can see it

**Option 9 - View Chat History:**
- Shows last 50 messages in project
- Displays: sender name, message, timestamp
- Ordered chronologically

**Option 10 - Add Contact:**
- Enter: username or email
- Adds user as contact (friend)

**Option 11 - View Contacts:**
- Shows all your contacts
- Displays: username, email, status

**Option 12 - Logout:**
- Clears your session
- Back to not logged in

**Option 0 - Exit:**
- Closes the client program

---

## How It Works

### Architecture:

```
┌──────────────┐         TCP Socket (Port 8080)         ┌──────────────┐
│              │         Binary Protocol + JSON          │              │
│  Client 1    │◄─────────────────────────────────────►│    Server    │
│  (Terminal)  │                                         │  (Terminal)  │
│              │                                         │              │
└──────────────┘                                         │              │
                                                         │              │
┌──────────────┐                                         │              │
│              │                                         │              │
│  Client 2    │◄─────────────────────────────────────►│              │
│  (Terminal)  │         TCP Socket (Port 8080)         │              │
│              │         Binary Protocol + JSON          │              │
└──────────────┘                                         └──────┬───────┘
                                                                │
                                                                │
                                                                ▼
                                                       ┌─────────────────┐
                                                       │   PostgreSQL    │
                                                       │    Database     │
                                                       │                 │
                                                       │  - Users        │
                                                       │  - Projects     │
                                                       │  - Tasks        │
                                                       │  - Chat Logs    │
                                                       │  - Contacts     │
                                                       └─────────────────┘
```

### How Messages Work:

1. **Client sends request:**
   - Header (16 bytes): type, size, version
   - Payload (JSON): {"username":"john", "password":"pass"}

2. **Server processes:**
   - Routes to appropriate service (auth, project, task, etc.)
   - Queries database
   - Generates response

3. **Server sends response:**
   - Header (16 bytes): type, size, version
   - Payload (JSON): {"status":0, "user_id":"...", "username":"john"}

4. **Client displays result:**
   - Parses JSON response
   - Shows success/error to user

### Protocol Example (Login):

```
Client -> Server:
  Type: MSG_LOGIN_REQUEST
  {"username":"john","password":"password123"}

Server -> Client:
  Type: MSG_LOGIN_RESPONSE
  {"status":0,"user_id":"user-001","email":"john@example.com"}

Client -> Server:
  Type: MSG_OTP_VERIFY_REQUEST
  {"email":"john@example.com","otp_code":"123456","otp_type":"login"}

Server -> Client:
  Type: MSG_OTP_VERIFY_RESPONSE
  {"status":0,"user_id":"user-001","username":"john","email":"john@example.com"}
```

---

## API Reference (For Your UI)

If you want to integrate the backend with your custom UI instead of the terminal client:

### Include Headers:

```cpp
#include "Client/network_manager.h"
#include "Client/session_manager.h"
#include "Common/protocol.h"
#include "Common/json_helper.h"
```

### Create Instances:

```cpp
NetworkManager g_network("127.0.0.1", 8080);
SessionManager g_session;
```

### Connect and Login:

```cpp
// Connect to server
if (!g_network.connect()) {
    std::cout << "Cannot connect!" << std::endl;
    return;
}

// Login
std::string response;
g_network.login("john", "password123", response);

// Parse response
JsonParser parser(response);
std::string email = parser.getString("email");

// Verify OTP
g_network.verifyOTP(email, "123456", "login", response);

// Save session
parser = JsonParser(response);
UserInfo user;
user.user_id = parser.getString("user_id");
user.username = parser.getString("username");
user.email = parser.getString("email");
g_session.setCurrentUser(user);
```

### Available Methods:

**Authentication:**
```cpp
bool registerUser(username, email, password, response);
bool login(username, password, response);
bool verifyOTP(email, otp_code, otp_type, response);
bool logout(response);
```

**Projects:**
```cpp
bool getProjects(response);
bool createProject(name, description, response);
bool updateProject(id, name, description, status, response);
bool deleteProject(id, response);
bool getProjectDetails(id, response);
bool inviteToProject(project_id, username, response);
```

**Tasks:**
```cpp
bool getTasks(project_id, response);
bool createTask(project_id, name, desc, assigned_to, priority, due_date, response);
bool updateTask(id, name, desc, assigned_to, status, priority, due_date, response);
bool deleteTask(id, response);
```

**Chat:**
```cpp
bool sendChatMessage(project_id, message, response);
bool getChatHistory(project_id, limit, response);
```

**Contacts:**
```cpp
bool addContact(username_or_email, response);
bool getContacts(response);
```

**Files:**
```cpp
bool uploadFile(task_id, name, path, type, size, response);
bool getFiles(task_id, response);
```

---

## Troubleshooting

### Problem: Server won't start

**Error:** `Cannot bind socket` or `Address already in use`

**Solution:**
```bash
# Check if something is on port 8080
lsof -i :8080

# Kill it if needed
sudo kill <PID>

# Or restart server
pkill server
./build/server
```

**Error:** `Database connection failed`

**Solution:**
```bash
# Check PostgreSQL is running
sudo systemctl status postgresql
sudo systemctl start postgresql

# Re-setup database if needed
cd DB
./setup_database.sh
cd ..
```

---

### Problem: Client can't connect

**Error:** `Cannot connect to server!`

**Solution:**
1. Make sure server is running:
   ```bash
   ps aux | grep server
   ```

2. If not running:
   ```bash
   ./build/server
   ```

3. Check server shows "Server listening on port 8080"

---

### Problem: OTP issues

**Issue:** Can't see OTP code

**Solution:**
- OTP is printed in **SERVER terminal (Terminal 1)**
- Look for this output:
  ```
  ========== EMAIL SERVICE ==========
  OTP Code: 123456
  ===================================
  ```
- **Keep Terminal 1 visible during demo!**

**Issue:** Invalid OTP error

**Solution:**
- OTP expires in 10 minutes
- Each OTP can only be used once
- Make sure you're using the most recent OTP from server terminal

---

### Problem: Can't find project ID

**Solution:**
```
In client, choose option: 3 (Get My Projects)
Look at the output, find: "project_id":"proj-xxxxx..."
Copy that entire ID (it's a UUID)
Use it in option 5, 8, or 9
```

---

### Problem: Need to start fresh

**Solution:**
```bash
# Stop everything
pkill server
pkill client

# Reset database
cd DB
./setup_database.sh
./load_sample_data.sh  # Optional
cd ..

# Restart
./build/server
```

---

### Problem: Compilation errors

**Solution:**
```bash
# Install dependencies
sudo apt update
sudo apt install g++ libpq-dev libssl-dev postgresql

# Clean and rebuild
make clean
make all
```

---

## Quick Commands Reference

### Build:
```bash
make all          # Build both server and client
make server       # Build server only
make client       # Build client only
make clean        # Clean build files
```

### Run:
```bash
./build/server    # Run server
./build/client    # Run client (in another terminal)
```

### Check Status:
```bash
ps aux | grep server    # Check if server is running
ps aux | grep client    # Check if clients are running
lsof -i :8080           # Check what's on port 8080
```

### Logs:
```bash
tail -f server.log      # Watch server log
tail -f client.log      # Watch client log
cat server.log          # View full server log
```

### Database:
```bash
# Connect to database
psql -U netprog_user -d netprog_db -h localhost
# Password: netprog_password

# Inside psql:
SET search_path TO netprog;
SELECT * FROM users;
SELECT * FROM projects;
SELECT * FROM tasks;
SELECT * FROM project_chat_log;
\q
```

### Kill Everything:
```bash
pkill server
pkill client
```

---

## Project Structure

```
HUST-NetworkProgramming-TaskManagement/
│
├── Server/                      # Backend server
│   ├── server.cpp              # Main TCP server ⭐
│   ├── database_handler.h      # PostgreSQL interface
│   ├── email_service.h         # OTP service
│   ├── auth_service.h          # Authentication
│   ├── project_service.h       # Project CRUD
│   ├── task_service.h          # Task CRUD
│   ├── chat_service.h          # Chat messaging
│   ├── file_service.h          # File attachments
│   └── contact_service.h       # Friend management
│
├── Client/                     # Client-side
│   ├── client.cpp              # Interactive client ⭐
│   ├── network_manager.h       # TCP client API
│   └── session_manager.h       # State management
│
├── Common/                     # Shared code
│   ├── protocol.h              # Message types, status codes
│   ├── json_helper.h           # JSON utilities
│   └── message_logger.h        # Logging
│
├── DB/                         # Database
│   ├── schema.sql              # Database schema
│   ├── sample_data.sql         # Test data
│   ├── setup_database.sh       # Setup script
│   └── load_sample_data.sh     # Data loader
│
├── build/                      # Compiled binaries
│   ├── server                  # Server executable ⭐
│   └── client                  # Client executable ⭐
│
├── Makefile                    # Build system
└── README.md                   # This file
```

---

## Demo Tips

### During Your Presentation:

1. **Arrange terminals:**
   - Terminal 1 (top): Server - keep visible for OTP codes
   - Terminal 2 (bottom-left): Client 1
   - Terminal 3 (bottom-right): Client 2

2. **What to explain:**
   - "We have a multi-threaded TCP server handling multiple clients"
   - "Communication uses binary protocol with JSON payloads"
   - "All data is stored in PostgreSQL database"
   - "OTP verification adds security to authentication"
   - "Real-time chat and task updates between clients"

3. **Show the flow:**
   - Start server first
   - Connect both clients
   - Both users login
   - One creates project
   - Both collaborate on tasks
   - Show chat working between them
   - Update task status, show it reflects on other client

4. **Highlight features:**
   - Multiple clients simultaneously
   - Real-time updates
   - Persistent database storage
   - Secure authentication with OTP
   - Project and task management
   - Team collaboration

---

## Final Checklist

Before your demo:

- [ ] Database is setup (`cd DB && ./setup_database.sh`)
- [ ] Server and client are built (`make all`)
- [ ] Test login with one client
- [ ] Arrange 3 terminals side-by-side
- [ ] Practice the demo flow once
- [ ] Prepare to explain architecture

During demo:

- [ ] Start server first, show it listening
- [ ] Start 2 clients
- [ ] Register/login on both
- [ ] Create project on client 1
- [ ] Show collaboration (chat, tasks)
- [ ] Update task, show real-time sync
- [ ] Explain technical details while showing

---

## You're Ready! 🚀

**Read this file** - it has everything you need!

**Run these 3 commands** in 3 terminals:
1. `./build/server`
2. `./build/client`
3. `./build/client`

**Then follow the demo script above!**

Good luck with your presentation! 🎉
