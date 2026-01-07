# Database Setup Guide

## Overview
This guide will help you set up the PostgreSQL database for the Network Programming Task Management System.

## Improvements Made to the Database Schema

Based on the requirements in `document.txt`, the following improvements were made to the original `database.txt` schema:

### ✅ New Tables Added:
1. **otp_verifications** - Stores OTP codes for email verification during login/registration
2. **user_contacts** - Manages friend/contact relationships between users (mentioned in requirements)
3. **notifications** - Stores user notifications for collaborative features

### ✅ Enhancements:
1. **Indexes** - Added indexes on all foreign keys and frequently queried columns for better performance
2. **Constraints** - Added unique constraints and check constraints for data integrity
3. **Triggers** - Added `updated_at` triggers to automatically track record modifications
4. **Priority field** - Added priority field to tasks table (low, medium, high, urgent)
5. **Cascading deletes** - Proper CASCADE and SET NULL behaviors for referential integrity
6. **Comments** - Added documentation comments to schema and tables

### 📊 Database Schema Structure:

```
users (authentication & profiles)
├── otp_verifications (OTP codes)
├── user_contacts (friend system)
├── notifications (user notifications)
└── projects (user's projects)
    ├── project_members (members & roles)
    ├── project_chat_log (project chat)
    └── tasks
        ├── task_comments
        └── file_attachments
```

## Quick Setup (Automated)

### Option 1: Run the setup script
```bash
cd Documents
./setup_database.sh
```

This will automatically:
- Create database user `netprog_user`
- Create database `netprog_db`
- Create schema `netprog` with all tables
- Set up proper permissions
- Generate connection information file

## Manual Setup

### Option 2: Step-by-step manual setup

#### 1. Switch to postgres user and create database:
```bash
sudo -u postgres psql
```

#### 2. In PostgreSQL shell, run:
```sql
-- Create user
CREATE USER netprog_user WITH PASSWORD 'your_secure_password';

-- Create database
CREATE DATABASE netprog_db OWNER netprog_user;

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE netprog_db TO netprog_user;
ALTER USER netprog_user CREATEDB;

-- Exit
\q
```

#### 3. Create the schema:
```bash
sudo -u postgres psql -d netprog_db -f schema.sql
```

#### 4. Grant permissions:
```bash
sudo -u postgres psql -d netprog_db
```

```sql
GRANT ALL PRIVILEGES ON SCHEMA netprog TO netprog_user;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA netprog TO netprog_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA netprog TO netprog_user;
\q
```

## Verify Installation

### Check if tables were created:
```bash
psql -U netprog_user -d netprog_db
```

```sql
SET search_path TO netprog;
\dt
```

You should see 11 tables:
- users
- otp_verifications
- user_contacts
- projects
- project_members
- tasks
- task_comments
- project_chat_log
- file_attachments
- notifications

### View table structure:
```sql
\d+ users
\d+ tasks
\d+ projects
```

## Load Sample Data (Optional)

For testing and development, you can load sample data that includes:
- 8 users with different roles
- 6 projects (including completed, in-progress, and planning stages)
- 20 tasks with various statuses and priorities
- Chat messages, comments, file attachments, and notifications

### Load sample data:
```bash
cd Documents
./load_sample_data.sh
```

Or manually:
```bash
psql -U netprog_user -d netprog_db -f sample_data.sql
```

See `SAMPLE_DATA_GUIDE.md` for detailed information about the sample data and useful queries.

## Connection Information

### Connection String for Application:
```
postgresql://netprog_user:your_password@localhost:5432/netprog_db
```

### psql Connection:
```bash
psql -U netprog_user -d netprog_db
```

### Python Connection Example:
```python
import psycopg2

conn = psycopg2.connect(
    dbname="netprog_db",
    user="netprog_user",
    password="your_password",
    host="localhost",
    port="5432"
)

# Set schema
cur = conn.cursor()
cur.execute("SET search_path TO netprog;")
```

### Java (JDBC) Connection Example:
```java
String url = "jdbc:postgresql://localhost:5432/netprog_db?currentSchema=netprog";
String user = "netprog_user";
String password = "your_password";

Connection conn = DriverManager.getConnection(url, user, password);
```

### Node.js Connection Example:
```javascript
const { Pool } = require('pg');

const pool = new Pool({
    host: 'localhost',
    port: 5432,
    database: 'netprog_db',
    user: 'netprog_user',
    password: 'your_password'
});

// Set schema for each query
await pool.query('SET search_path TO netprog');
```

## Database Schema Alignment with Requirements

### ✅ Login/Signup with OTP (document.txt lines 74-76)
- `users` table stores user credentials
- `otp_verifications` table manages OTP codes
- `is_verified` field tracks verification status

### ✅ Project Management (lines 83-90)
- `projects` table stores project info
- `project_members` table with `role` field (owner marked by role='owner')
- Owner has full permissions (managed at application level)

### ✅ Task CRUD (lines 92-105)
- `tasks` table with all required fields:
  - task_name (task column)
  - assigned_to (people column)
  - status (status column: todo, in_progress, completed)
  - start_date & due_date (deadline columns)
- `file_attachments` table for file management

### ✅ Chat Functionality (lines 107-109)
- `project_chat_log` table stores chat history
- Indexed by created_at for efficient retrieval

### ✅ Friend System (lines 80)
- `user_contacts` table manages friend relationships
- Status field: pending, accepted, blocked

### ✅ User Settings (lines 115-119)
- `users` table contains username, email, password
- Can be extended with profile information

## Troubleshooting

### Issue: "Peer authentication failed"
**Solution:** Edit PostgreSQL config to allow password authentication:
```bash
sudo nano /etc/postgresql/14/main/pg_hba.conf
```
Change:
```
local   all   postgres   peer
```
To:
```
local   all   postgres   md5
```
Restart PostgreSQL:
```bash
sudo systemctl restart postgresql
```

### Issue: "Role does not exist"
**Solution:** Create the user first:
```bash
sudo -u postgres createuser -P netprog_user
```

### Issue: "Permission denied for schema"
**Solution:** Grant permissions:
```sql
sudo -u postgres psql -d netprog_db
GRANT ALL ON SCHEMA netprog TO netprog_user;
```

## Security Recommendations

1. **Change default password** - Use a strong password in production
2. **Add to .gitignore** - Don't commit `db_connection_info.txt`
3. **Use environment variables** - Store credentials in `.env` file
4. **Enable SSL** - For production deployments
5. **Regular backups** - Use `pg_dump` for backups

## Backup and Restore

### Backup:
```bash
pg_dump -U netprog_user -d netprog_db -n netprog -f backup.sql
```

### Restore:
```bash
psql -U netprog_user -d netprog_db -f backup.sql
```

## Next Steps

1. Update `.gitignore` to exclude `db_connection_info.txt`
2. Configure your application's database connection
3. Test CRUD operations
4. Set up database migrations (optional)
5. Configure connection pooling for production

## Support

For issues or questions, refer to:
- PostgreSQL documentation: https://www.postgresql.org/docs/
- Project requirements: `document.txt`
- Database schema: `schema.sql`
