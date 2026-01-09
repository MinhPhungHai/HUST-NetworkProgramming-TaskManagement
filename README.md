# Task Management Application

**Network Programming Final Project - Multi-threaded TCP Client-Server Application with GTK+ UI**

---

## Quick Start (2 Terminals)

### Terminal 1: Start Server

```bash
cd /home/mp/NPFInalPRJ/HUST-NetworkProgramming-TaskManagement
make
./build/server
```

**Expected Output:**
```
===== Task Management Server =====
Database connected successfully
Server listening on port 8080
```

### Terminal 2: Start GTK UI

```bash
cd /home/mp/NPFInalPRJ/HUST-NetworkProgramming-TaskManagement/UI
make -f Makefile.integrated
env -i HOME=$HOME DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY PATH=/usr/bin:/bin ./build/start_integrated
```

**Important Notes:**
- The UI connects to `127.0.0.1:8080` (localhost)
- **OTP codes appear in the server terminal** - keep it visible!
- Always use the `env -i ...` command to avoid Snap library conflicts

---

## Prerequisites

### System Requirements

- **OS:** Ubuntu/Debian Linux
- **Compiler:** g++ with C++17 support
- **Build Tools:** make
- **Database:** PostgreSQL

### Required Packages

Install all dependencies:

```bash
sudo apt update
sudo apt install -y g++ make libpq-dev libssl-dev postgresql \
    libgtk-3-dev nlohmann-json3-dev
```

### Database Setup

1. **Setup PostgreSQL:**
   ```bash
   cd DB
   ./setup_database.sh
   ```

2. **Load sample data (optional):**
   ```bash
   ./load_sample_data.sh
   ```

**Default Database Configuration:**
- Database: `netprog_db`
- User: `netprog_user`
- Password: `netprog_password`
- Schema: `netprog`

---

## Application Features

### ✅ Complete Features

**Authentication & Security:**
- User registration with OTP verification
- Secure login with password hashing (SHA256)
- Session management
- OTP-based verification (displayed in server terminal)

**Project Management:**
- Create and manage projects
- Invite team members to projects
- Project owner permissions
- View all your projects

**Task Management:**
- Create tasks with deadlines and priorities
- Assign tasks to team members
- Update task status (To Do, In Progress, Completed, In Review, Blocked)
- Set priority levels (Low, Medium, High, Urgent)
- Delete tasks
- Tasks sorted by priority and deadline
- Gantt chart visualization

**Team Collaboration:**
- Real-time project chat
- View chat history
- Auto-refresh chat messages (every 2 seconds)
- Contact/friend management

**File Management:**
- Attach file links to tasks
- Click links to open in browser (Firefox/OperaGX)
- Multiple file attachments per task

---

## Architecture

```
┌────────────────┐         TCP Socket (Port 8080)         ┌────────────────┐
│                │         Binary Protocol + JSON          │                │
│  GTK+ UI       │◄─────────────────────────────────────►│  Multi-threaded│
│  Client        │                                         │  TCP Server    │
│                │                                         │                │
└────────────────┘                                         └────────┬───────┘
                                                                    │
                                                                    ▼
                                                          ┌──────────────────┐
                                                          │   PostgreSQL     │
                                                          │   Database       │
                                                          │                  │
                                                          │  - Users (auth)  │
                                                          │  - Projects      │
                                                          │  - Tasks         │
                                                          │  - Chat logs     │
                                                          │  - Files         │
                                                          │  - Contacts      │
                                                          └──────────────────┘
```

### Technology Stack

**Server:**
- C++17 with STL
- Multi-threaded TCP socket programming
- PostgreSQL database (libpq)
- OpenSSL for password hashing
- Binary message protocol with JSON payloads

**Client (GTK UI):**
- GTK+ 3.0 GUI framework
- C++17 backend
- Network wrapper for TCP communication
- nlohmann/json for JSON parsing
- Cairo graphics for Gantt chart

---

## Building the Project

### Build Server

```bash
# From project root
make            # Build both server and client
make server     # Build server only
make clean      # Clean build files
```

### Build GTK UI

```bash
# From UI directory
cd UI
make -f Makefile.integrated clean
make -f Makefile.integrated
```

---

## Project Structure

```
HUST-NetworkProgramming-TaskManagement/
│
├── Server/                     # Backend server
│   ├── server.cpp             # Main TCP server
│   ├── database_handler.h     # PostgreSQL interface
│   ├── email_service.h        # OTP service
│   ├── auth_service.h         # Authentication
│   ├── project_service.h      # Project management
│   ├── task_service.h         # Task management
│   ├── chat_service.h         # Chat messaging
│   ├── file_service.h         # File attachments
│   └── contact_service.h      # Friend management
│
├── Client/                    # Network client library
│   ├── network_manager.h      # TCP client API
│   ├── network_wrapper.h      # C wrapper for GTK
│   └── session_manager.h      # State management
│
├── UI/                        # GTK+ User Interface
│   ├── start_integrated.cpp   # Main entry point
│   ├── login_integrated.cpp   # Login screen
│   ├── register.cpp           # Registration screen
│   ├── project_list.cpp       # Project list view
│   ├── project_view.cpp       # Project detail view
│   ├── settings.cpp           # Settings screen
│   ├── gantt.cpp              # Gantt chart view
│   └── Makefile.integrated    # GTK build file
│
├── Common/                    # Shared code
│   ├── protocol.h             # Message types & status codes
│   ├── json_helper.h          # JSON utilities
│   └── message_logger.h       # Logging
│
├── DB/                        # Database
│   ├── schema.sql             # Database schema (10 tables)
│   ├── sample_data.sql        # Test data
│   ├── setup_database.sh      # Setup script
│   └── load_sample_data.sh    # Data loader
│
├── build/                     # Compiled binaries
│   ├── server                 # Server executable
│   └── client                 # Terminal client (optional)
│
├── Makefile                   # Server build file
└── README.md                  # This file
```

---

## Usage Guide

### First Time Setup

1. **Start the server** (Terminal 1)
2. **Launch the UI** (Terminal 2)
3. **Register a new account:**
   - Enter username, email, password
   - Check **server terminal** for OTP code
   - Enter OTP in the UI
4. **Login:**
   - Enter username and password
   - Check **server terminal** for OTP code
   - Enter OTP in the UI

### Working with Projects

1. **Create a project:**
   - Click "➕ Create Project"
   - Enter project name and description

2. **View project details:**
   - Click on a project card
   - View tasks, chat, and members

3. **Manage tasks:**
   - Click "Add Task" to create new tasks
   - Set priority, deadline, and assign to members
   - Tasks are automatically sorted by priority and deadline
   - Click "Edit" to modify task details
   - Update status by clicking the status dropdown

4. **Team collaboration:**
   - Use the chat panel on the right side
   - Messages auto-refresh every 2 seconds
   - Add members to projects via "Add Member" button

5. **Gantt chart:**
   - Click "📊 Gantt Chart" to visualize tasks timeline
   - Dates displayed in DD/MM/YYYY format

6. **File attachments:**
   - Click "Add File" on a task
   - Paste file link (Google Drive, Dropbox, etc.)
   - Click "File" in task list to open link in browser

---

## Troubleshooting

### Server Issues

**Error: `Cannot bind socket` or `Address already in use`**

```bash
# Check what's using port 8080
lsof -i :8080

# Kill the process
sudo kill <PID>

# Or restart server
pkill server
./build/server
```

**Error: `Database connection failed`**

```bash
# Check PostgreSQL status
sudo systemctl status postgresql
sudo systemctl start postgresql

# Re-setup database
cd DB
./setup_database.sh
```

### UI Issues

**Error: UI doesn't start or crashes**

Always run with the `env -i` command:
```bash
env -i HOME=$HOME DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY PATH=/usr/bin:/bin ./build/start_integrated
```

**Issue: Windows don't appear or disappear**

This has been fixed in the latest version. Make sure you rebuild:
```bash
cd UI
make -f Makefile.integrated clean
make -f Makefile.integrated
```

**Issue: Can't find OTP code**

- OTP codes are printed in the **server terminal** (Terminal 1)
- Look for:
  ```
  ========== EMAIL SERVICE ==========
  OTP Code: 123456
  ===================================
  ```
- Keep the server terminal visible during login/registration

**Issue: Chat not updating**

- Chat auto-refreshes every 2 seconds
- Make sure the server is running
- Check network connection

**Issue: nlohmann/json.hpp not found**

```bash
sudo apt install nlohmann-json3-dev
```

### Build Issues

**Compilation errors:**

```bash
# Install all dependencies
sudo apt update
sudo apt install g++ make libpq-dev libssl-dev postgresql \
    libgtk-3-dev nlohmann-json3-dev

# Clean and rebuild
cd UI
make -f Makefile.integrated clean
make -f Makefile.integrated
```

---

## Database Management

### Connect to Database

```bash
psql -U netprog_user -d netprog_db -h localhost
# Password: netprog_password
```

### Useful SQL Commands

```sql
-- Set schema
SET search_path TO netprog;

-- View users
SELECT * FROM users;

-- View projects
SELECT * FROM projects;

-- View tasks
SELECT * FROM tasks;

-- View chat messages
SELECT * FROM project_chat_log ORDER BY created_at DESC LIMIT 20;

-- View contacts
SELECT * FROM user_contacts;

-- Exit
\q
```

### Reset Database

```bash
cd DB
./setup_database.sh
./load_sample_data.sh  # Optional: load test data
```

---

## Development

### Adding New Features

1. **Server side:**
   - Add service handler in `Server/`
   - Update protocol in `Common/protocol.h`
   - Add database queries in service

2. **Client side (UI):**
   - Add UI components in `UI/`
   - Use network wrapper functions in `Client/network_wrapper.h`
   - Update UI state and display

### Testing

```bash
# Kill all processes
pkill server
pkill start_integrated

# Restart fresh
./build/server
cd UI && ./build/start_integrated
```

---

## Network Protocol

### Message Format

```
┌─────────────────────────────────────┐
│  Header (16 bytes)                  │
│  - Message Type (4 bytes)           │
│  - Payload Size (4 bytes)           │
│  - Version (4 bytes)                │
│  - Reserved (4 bytes)               │
├─────────────────────────────────────┤
│  JSON Payload (variable size)       │
│  Example: {"username":"john",...}   │
└─────────────────────────────────────┘
```

### Message Types

- Authentication: `MSG_LOGIN_REQUEST`, `MSG_REGISTER_REQUEST`, `MSG_OTP_VERIFY_REQUEST`
- Projects: `MSG_GET_PROJECTS_REQUEST`, `MSG_CREATE_PROJECT_REQUEST`
- Tasks: `MSG_GET_TASKS_REQUEST`, `MSG_CREATE_TASK_REQUEST`, `MSG_UPDATE_TASK_REQUEST`
- Chat: `MSG_SEND_CHAT_REQUEST`, `MSG_GET_CHAT_HISTORY_REQUEST`
- Files: `MSG_UPLOAD_FILE_REQUEST`, `MSG_GET_FILES_REQUEST`

---

## Contributors

This project was developed as a Network Programming final project demonstrating:
- Multi-threaded TCP socket programming
- Client-server architecture
- Database integration
- Real-time communication
- GUI development with GTK+

---

## License

Educational project for Network Programming course.
