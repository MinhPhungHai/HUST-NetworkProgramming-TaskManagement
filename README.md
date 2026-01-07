# WorkApp – Network Programming Project

Task Management Application with PostgreSQL Database

---

## 📋 Overview

**WorkApp** is a project management application built using a **client-server architecture** with **TCP socket programming**. This repository contains the complete PostgreSQL database setup with automated installation scripts.

### Features

- User authentication with OTP email verification
- Project creation and management
- Task assignment and tracking with multiple statuses
- Real-time chat within projects
- File attachments for tasks
- Friend/contact system
- User notifications
- Role-based access control (owner, admin, member, viewer)

---

## 🚀 Quick Database Setup

### Prerequisites

```bash
# Install PostgreSQL
sudo apt update
sudo apt install postgresql postgresql-contrib
```

### Setup (2 commands)

```bash
cd DB

# 1. Setup database (creates everything)
./setup_database.sh

# 2. Load sample data (optional, for testing)
./load_sample_data.sh
```

That's it! Your database is ready.

---

## 📊 Database Details

### Created Resources

- **Database:** `netprog_db`
- **User:** `netprog_user`
- **Password:** `netprog_password` (change in production!)
- **Schema:** `netprog`

### Database Schema (10 Tables)

```
users                    # User accounts and authentication
├── otp_verifications    # OTP codes for login/registration
├── user_contacts        # Friend/contact relationships
├── notifications        # User notifications
└── projects             # User's projects
    ├── project_members  # Project members with roles
    ├── project_chat_log # Project chat messages
    └── tasks            # Tasks within projects
        ├── task_comments      # Comments on tasks
        └── file_attachments   # File attachments
```

### Performance Features

- ✅ **38 indexes** on foreign keys and frequently queried columns
- ✅ **4 automatic triggers** for timestamp updates
- ✅ **Foreign key constraints** with proper cascading deletes
- ✅ **Check constraints** for data validation
- ✅ **Unique constraints** to prevent duplicates

---

## 🔌 Connecting to the Database

### psql Command Line

```bash
psql -U netprog_user -d netprog_db -h localhost
```

Once connected:
```sql
-- Set the schema
SET search_path TO netprog;

-- List all tables
\dt

-- View table structure
\d users
\d tasks
\d projects

-- Sample queries
SELECT * FROM users;
SELECT * FROM projects;
SELECT * FROM tasks;
```

### Connection String

```
postgresql://netprog_user:netprog_password@localhost:5432/netprog_db
```

### Python (psycopg2)

```python
import psycopg2

conn = psycopg2.connect(
    dbname="netprog_db",
    user="netprog_user",
    password="netprog_password",
    host="localhost",
    port="5432"
)

# Set schema
cur = conn.cursor()
cur.execute("SET search_path TO netprog;")

# Query
cur.execute("SELECT * FROM users;")
users = cur.fetchall()
```

### Java (JDBC)

```java
String url = "jdbc:postgresql://localhost:5432/netprog_db?currentSchema=netprog";
String user = "netprog_user";
String password = "netprog_password";

Connection conn = DriverManager.getConnection(url, user, password);

// Query
Statement stmt = conn.createStatement();
ResultSet rs = stmt.executeQuery("SELECT * FROM users");
```

### Node.js (pg)

```javascript
const { Pool } = require('pg');

const pool = new Pool({
    host: 'localhost',
    port: 5432,
    database: 'netprog_db',
    user: 'netprog_user',
    password: 'netprog_password'
});

// Set schema
await pool.query('SET search_path TO netprog');

// Query
const result = await pool.query('SELECT * FROM users');
console.log(result.rows);
```

### C (libpq)

```c
#include <libpq-fe.h>

PGconn *conn = PQconnectdb(
    "host=localhost port=5432 dbname=netprog_db "
    "user=netprog_user password=netprog_password"
);

// Set schema
PQexec(conn, "SET search_path TO netprog;");

// Query
PGresult *res = PQexec(conn, "SELECT * FROM users");
```

---

## 📦 Sample Data

The `load_sample_data.sh` script provides realistic test data:

### Users (8 accounts)
- john_doe, jane_smith, mike_wilson, sarah_johnson, david_brown, emily_davis, alex_nguyen, lisa_tran
- All passwords: `password123` (bcrypt hashed)

### Projects (6 projects)
- Network Programming Final Project (in_progress)
- E-Commerce Website (in_progress)
- Fitness Tracking App (planning)
- Database Migration Tool (completed)
- Image Recognition System (planning)
- Personal Portfolio Website (on_hold)

### Tasks (20 tasks)
- 5 completed
- 6 in progress
- 6 todo
- 1 in review
- 1 blocked
- Various priorities: urgent, high, medium, low

### Additional Data
- 15 task comments with technical discussions
- 27 chat messages showing project collaboration
- 14 file attachments (various types)
- 19 notifications (assignments, comments, invites)
- 14 friend connections (accepted, pending, blocked)
- 6 OTP verification records

---

## 💡 Useful SQL Queries

### View all projects with member count

```sql
SELECT
    p.project_name,
    u.username AS owner,
    p.status,
    COUNT(DISTINCT pm.user_id) AS members,
    COUNT(DISTINCT t.task_id) AS tasks
FROM projects p
JOIN users u ON p.owner_id = u.user_id
LEFT JOIN project_members pm ON p.project_id = pm.project_id
LEFT JOIN tasks t ON p.project_id = t.project_id
GROUP BY p.project_id, p.project_name, u.username, p.status
ORDER BY p.created_at;
```

### View tasks with assignees

```sql
SELECT
    t.task_name,
    p.project_name,
    u1.username AS assigned_to,
    u2.username AS created_by,
    t.status,
    t.priority,
    t.due_date
FROM tasks t
JOIN projects p ON t.project_id = p.project_id
LEFT JOIN users u1 ON t.assigned_to = u1.user_id
JOIN users u2 ON t.created_by = u2.user_id
ORDER BY t.due_date;
```

### View project chat history

```sql
SELECT
    u.username,
    pc.message_content,
    pc.created_at
FROM project_chat_log pc
JOIN users u ON pc.sender_id = u.user_id
WHERE pc.project_id = 'proj-001'
ORDER BY pc.created_at;
```

### View user's unread notifications

```sql
SELECT
    notification_type,
    message,
    created_at
FROM notifications
WHERE user_id = 'user-001' AND is_read = false
ORDER BY created_at DESC;
```

### Project progress statistics

```sql
SELECT
    p.project_name,
    COUNT(t.task_id) AS total_tasks,
    COUNT(CASE WHEN t.status = 'completed' THEN 1 END) AS completed,
    COUNT(CASE WHEN t.status = 'in_progress' THEN 1 END) AS in_progress,
    COUNT(CASE WHEN t.status = 'todo' THEN 1 END) AS todo,
    ROUND(100.0 * COUNT(CASE WHEN t.status = 'completed' THEN 1 END) /
          NULLIF(COUNT(t.task_id), 0), 2) AS completion_percent
FROM projects p
LEFT JOIN tasks t ON p.project_id = t.project_id
GROUP BY p.project_id, p.project_name
ORDER BY p.project_name;
```

---

## 🔧 Troubleshooting

### Can't connect to database?

1. Check PostgreSQL is running:
```bash
sudo systemctl status postgresql
```

2. Restart PostgreSQL:
```bash
sudo systemctl restart postgresql
```

3. Check authentication config:
```bash
sudo cat /etc/postgresql/*/main/pg_hba.conf | grep netprog
```

Should see:
```
local   netprog_db      netprog_user                            md5
host    netprog_db      netprog_user    127.0.0.1/32            md5
```

### Permission errors?

Connect as postgres user and grant permissions:
```bash
sudo -u postgres psql -d netprog_db
```

```sql
GRANT ALL PRIVILEGES ON SCHEMA netprog TO netprog_user;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA netprog TO netprog_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA netprog TO netprog_user;
```

### Want to reset everything?

```bash
# Drop the database
sudo -u postgres psql -c "DROP DATABASE netprog_db;"

# Run setup again
cd DB
./setup_database.sh
./load_sample_data.sh
```

---

## 📁 Project Structure

```
DB/
├── schema.sql              # Database schema (10 tables, indexes, triggers)
├── sample_data.sql         # Sample data (100+ records)
├── setup_database.sh       # Complete database setup script
├── load_sample_data.sh     # Load sample data script
├── database.txt            # Original DBML schema design
└── document.txt            # Project requirements (Vietnamese)
```

---

## 🔒 Security Notes

⚠️ **Important for Production:**

1. **Change the default password** from `netprog_password`
2. **Use environment variables** for credentials instead of hardcoding
3. **Enable SSL** for remote connections
4. **Restrict network access** in PostgreSQL config (`pg_hba.conf`)
5. **Don't commit** `db_connection_info.txt` to git (already in `.gitignore`)
6. **Use prepared statements** in your application to prevent SQL injection
7. **Hash passwords** with bcrypt (strength 10+)
8. **Validate all input** before database operations

---

## 📖 Database Schema Details

### users
Stores user account information.

| Column | Type | Description |
|--------|------|-------------|
| user_id | VARCHAR(36) | Primary key (UUID) |
| username | VARCHAR(255) | Unique username |
| email | VARCHAR(255) | Unique email |
| phone_number | VARCHAR(20) | Unique phone (optional) |
| password_hash | VARCHAR(255) | Bcrypt hashed password |
| is_verified | BOOLEAN | Email verification status |
| created_at | TIMESTAMP | Account creation time |

### projects
Stores project information.

| Column | Type | Description |
|--------|------|-------------|
| project_id | VARCHAR(36) | Primary key |
| project_name | VARCHAR(255) | Project name |
| description | TEXT | Project description |
| owner_id | VARCHAR(36) | FK to users (creator) |
| start_date | DATE | Project start date |
| end_date | DATE | Project end date |
| status | VARCHAR(20) | planning, in_progress, completed, on_hold |
| created_at | TIMESTAMP | Creation time |

### tasks
Stores tasks within projects.

| Column | Type | Description |
|--------|------|-------------|
| task_id | VARCHAR(36) | Primary key |
| project_id | VARCHAR(36) | FK to projects |
| task_name | VARCHAR(255) | Task name |
| description | TEXT | Task description |
| assigned_to | VARCHAR(36) | FK to users (assignee) |
| created_by | VARCHAR(36) | FK to users (creator) |
| status | VARCHAR(20) | todo, in_progress, review, completed, blocked |
| priority | VARCHAR(20) | low, medium, high, urgent |
| start_date | DATE | Task start date |
| due_date | DATE | Task deadline |
| completed_at | TIMESTAMP | Completion timestamp |
| created_at | TIMESTAMP | Creation time |

### project_chat_log
Stores chat messages within projects.

| Column | Type | Description |
|--------|------|-------------|
| chat_id | VARCHAR(36) | Primary key |
| project_id | VARCHAR(36) | FK to projects |
| sender_id | VARCHAR(36) | FK to users |
| message_content | TEXT | Chat message |
| created_at | TIMESTAMP | Message timestamp |

### otp_verifications
Stores OTP codes for email verification.

| Column | Type | Description |
|--------|------|-------------|
| otp_id | VARCHAR(36) | Primary key |
| user_id | VARCHAR(36) | FK to users |
| email | VARCHAR(255) | Email for OTP |
| otp_code | VARCHAR(10) | OTP code |
| otp_type | VARCHAR(20) | registration, login, password_reset |
| expires_at | TIMESTAMP | OTP expiration time |
| is_used | BOOLEAN | Whether OTP was used |
| created_at | TIMESTAMP | Creation time |

See `DB/schema.sql` for complete schema with all tables, indexes, and constraints.

---

## 🧪 Testing Your Application

1. **Use sample data** to test without manual setup
2. **Test user accounts** are ready (password: `password123`)
3. **Projects and tasks** are already created
4. **Test scenarios:**
   - User authentication (login/register/OTP)
   - Project CRUD operations
   - Task management
   - Chat functionality
   - File uploads
   - Notifications

---

## 📞 Support

For database setup issues:
1. Check this README first
2. Review the troubleshooting section
3. Check PostgreSQL logs: `sudo tail -f /var/log/postgresql/postgresql-14-main.log`
4. Verify PostgreSQL version: `psql --version`

---

## 📄 License

Part of HUST Network Programming Final Project.

---

**Ready to start building?** Your database is set up and waiting! 🚀
