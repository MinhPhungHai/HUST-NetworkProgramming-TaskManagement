# Sample Data Guide

## Overview
This guide describes the sample data included for testing and demonstration of the Network Programming Task Management System.

## How to Load Sample Data

### Quick Start:
```bash
cd Documents
./load_sample_data.sh
```

### Manual Load:
```bash
psql -U netprog_user -d netprog_db -f sample_data.sql
```

## Sample Data Contents

### 👥 Users (8 users)

| User ID | Username | Email | Phone | Verified | Role |
|---------|----------|-------|-------|----------|------|
| user-001 | john_doe | john.doe@example.com | +84901234567 | ✓ | Project owner |
| user-002 | jane_smith | jane.smith@example.com | +84901234568 | ✓ | Admin/Developer |
| user-003 | mike_wilson | mike.wilson@example.com | +84901234569 | ✓ | Developer |
| user-004 | sarah_johnson | sarah.johnson@example.com | +84901234570 | ✓ | Developer |
| user-005 | david_brown | david.brown@example.com | +84901234571 | ✓ | Viewer/Tester |
| user-006 | emily_davis | emily.davis@example.com | +84901234572 | ✗ | Not verified yet |
| user-007 | alex_nguyen | alex.nguyen@example.com | +84901234573 | ✓ | Developer |
| user-008 | lisa_tran | lisa.tran@example.com | +84901234574 | ✓ | Designer |

**Test Credentials:**
- All passwords are hashed version of: `password123`
- Use these usernames to test login functionality

### 📧 OTP Verifications (6 records)

Includes examples of:
- Used OTPs (login history)
- Pending OTP for unverified user
- Password reset OTP
- Different OTP types: registration, login, password_reset

### 👫 User Contacts (14 friend connections)

Friend network examples:
- **john_doe** is friends with: jane_smith, mike_wilson, sarah_johnson (pending: david_brown)
- **jane_smith** is friends with: john_doe, mike_wilson, alex_nguyen
- **mike_wilson** is friends with: john_doe, jane_smith (pending: lisa_tran)
- Various connection statuses: accepted, pending, blocked

### 📁 Projects (6 projects)

| Project ID | Name | Owner | Status | Members | Description |
|------------|------|-------|--------|---------|-------------|
| proj-001 | Network Programming Final Project | john_doe | in_progress | 5 | Main task management system |
| proj-002 | E-Commerce Website | jane_smith | in_progress | 3 | Full-stack e-commerce platform |
| proj-003 | Fitness Tracking App | mike_wilson | planning | 3 | Mobile fitness application |
| proj-004 | Database Migration Tool | john_doe | completed | 2 | Completed migration project |
| proj-005 | Image Recognition System | sarah_johnson | planning | 2 | CNN-based ML project |
| proj-006 | Personal Portfolio | david_brown | on_hold | 1 | Personal website project |

### 👥 Project Members (16 memberships)

Role distribution examples:
- **Owners**: Full control over their projects
- **Admins**: Can manage tasks and members
- **Members**: Can work on tasks
- **Viewers**: Read-only access

### ✅ Tasks (20 tasks)

Task status distribution:
- **Completed**: 5 tasks (including all tasks from proj-004)
- **In Progress**: 6 tasks
- **Todo**: 6 tasks
- **Review**: 1 task
- **Blocked**: 1 task

Priority levels:
- **High**: 10 tasks
- **Medium**: 6 tasks
- **Low**: 2 tasks
- **Urgent**: 1 task

#### Network Programming Project (proj-001) Tasks:

1. ✅ **Design Database Schema** (john_doe) - Completed
   - Priority: High
   - Description: Create comprehensive database schema

2. 🔄 **Implement Server Architecture** (jane_smith) - In Progress
   - Priority: High
   - Description: TCP server with multi-threading

3. 🔄 **Create Client Application** (mike_wilson) - In Progress
   - Priority: High
   - Description: GUI for task management

4. 📋 **Implement OTP Verification** (sarah_johnson) - Todo
   - Priority: Medium
   - Description: Email OTP for login/registration

5. 👀 **Design Protocol Messages** (jane_smith) - Review
   - Priority: High
   - Description: Define message structure and protocol

6. 📋 **Implement Chat Feature** (mike_wilson) - Todo
   - Priority: Medium
   - Description: Real-time chat functionality

7. 🚫 **Create Gantt Chart View** (sarah_johnson) - Blocked
   - Priority: Low
   - Description: Gantt chart visualization

8. 📋 **Write Unit Tests** (david_brown) - Todo
   - Priority: Medium
   - Description: Comprehensive unit tests

9. 📋 **Documentation** (john_doe) - Todo
   - Priority: Low
   - Description: Technical documentation

### 💬 Task Comments (15 comments)

Real conversation examples showing:
- Technical discussions
- Code review feedback
- Questions and answers
- Status updates
- Collaboration between team members

### 💭 Project Chat Log (27 messages)

Chat history includes:
- **proj-001**: 15 messages (team coordination, progress updates)
- **proj-002**: 5 messages (project planning)
- **proj-003**: 4 messages (team formation)
- **proj-005**: 3 messages (research discussion)

### 📎 File Attachments (14 files)

Various file types:
- SQL scripts (.sql)
- Images (.png)
- Documents (.pdf, .docx)
- Design files (.drawio, .fig, .psd, .sketch)
- Code archives (.zip)
- Data files (.json, .py, .xlsx)

### 🔔 Notifications (19 notifications)

Notification types:
- **task_assigned**: User assigned to new task
- **comment_added**: New comment on task
- **project_invite**: Invited to join project
- **task_updated**: Task status changed

Read status:
- 10 read notifications
- 9 unread notifications

## Sample Queries

### 1. View all active projects:
```sql
SET search_path TO netprog;

SELECT
    p.project_name,
    u.username AS owner,
    p.status,
    COUNT(DISTINCT pm.user_id) AS member_count,
    COUNT(DISTINCT t.task_id) AS task_count
FROM projects p
JOIN users u ON p.owner_id = u.user_id
LEFT JOIN project_members pm ON p.project_id = pm.project_id
LEFT JOIN tasks t ON p.project_id = t.project_id
WHERE p.status != 'completed'
GROUP BY p.project_id, p.project_name, u.username, p.status
ORDER BY p.created_at;
```

### 2. View tasks with details:
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

### 3. View project chat history:
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

### 4. View user's unread notifications:
```sql
SELECT
    notification_type,
    message,
    created_at
FROM notifications
WHERE user_id = 'user-001' AND is_read = false
ORDER BY created_at DESC;
```

### 5. View task comments:
```sql
SELECT
    t.task_name,
    u.username,
    tc.comment_content,
    tc.created_at
FROM task_comments tc
JOIN tasks t ON tc.task_id = t.task_id
JOIN users u ON tc.user_id = u.user_id
WHERE t.project_id = 'proj-001'
ORDER BY t.task_id, tc.created_at;
```

### 6. View user's friends:
```sql
SELECT
    u2.username AS friend_name,
    u2.email,
    uc.status,
    uc.created_at
FROM user_contacts uc
JOIN users u1 ON uc.user_id = u1.user_id
JOIN users u2 ON uc.contact_user_id = u2.user_id
WHERE u1.username = 'john_doe'
ORDER BY uc.created_at;
```

### 7. View project progress:
```sql
SELECT
    p.project_name,
    COUNT(t.task_id) AS total_tasks,
    COUNT(CASE WHEN t.status = 'completed' THEN 1 END) AS completed_tasks,
    COUNT(CASE WHEN t.status = 'in_progress' THEN 1 END) AS in_progress_tasks,
    COUNT(CASE WHEN t.status = 'todo' THEN 1 END) AS todo_tasks,
    ROUND(100.0 * COUNT(CASE WHEN t.status = 'completed' THEN 1 END) / NULLIF(COUNT(t.task_id), 0), 2) AS completion_percentage
FROM projects p
LEFT JOIN tasks t ON p.project_id = t.project_id
GROUP BY p.project_id, p.project_name
ORDER BY p.project_name;
```

### 8. View file attachments by task:
```sql
SELECT
    t.task_name,
    fa.file_name,
    fa.file_type,
    ROUND(fa.file_size / 1024.0, 2) AS size_kb,
    u.username AS uploaded_by,
    fa.created_at
FROM file_attachments fa
JOIN tasks t ON fa.task_id = t.task_id
JOIN users u ON fa.uploaded_by = u.user_id
ORDER BY t.task_name, fa.created_at;
```

### 9. View user activity summary:
```sql
SELECT
    u.username,
    COUNT(DISTINCT pm.project_id) AS projects_joined,
    COUNT(DISTINCT t1.task_id) AS tasks_assigned,
    COUNT(DISTINCT t2.task_id) AS tasks_created,
    COUNT(DISTINCT tc.comment_id) AS comments_made,
    COUNT(DISTINCT fa.file_id) AS files_uploaded
FROM users u
LEFT JOIN project_members pm ON u.user_id = pm.user_id
LEFT JOIN tasks t1 ON u.user_id = t1.assigned_to
LEFT JOIN tasks t2 ON u.user_id = t2.created_by
LEFT JOIN task_comments tc ON u.user_id = tc.user_id
LEFT JOIN file_attachments fa ON u.user_id = fa.uploaded_by
WHERE u.is_verified = true
GROUP BY u.user_id, u.username
ORDER BY projects_joined DESC;
```

### 10. View upcoming deadlines:
```sql
SELECT
    t.task_name,
    p.project_name,
    u.username AS assigned_to,
    t.due_date,
    t.status,
    CASE
        WHEN t.due_date < CURRENT_DATE THEN 'Overdue'
        WHEN t.due_date = CURRENT_DATE THEN 'Due Today'
        WHEN t.due_date <= CURRENT_DATE + 3 THEN 'Due Soon'
        ELSE 'On Track'
    END AS urgency
FROM tasks t
JOIN projects p ON t.project_id = p.project_id
LEFT JOIN users u ON t.assigned_to = u.user_id
WHERE t.status != 'completed'
ORDER BY t.due_date;
```

## Testing Scenarios

### 1. Login Flow Test
- User: `john_doe` (user-001)
- Email: `john.doe@example.com`
- Password: `password123` (hashed in database)
- Expected: OTP sent to email
- OTP records available in `otp_verifications` table

### 2. New User Registration Test
- User: `emily_davis` (user-006)
- Status: Not verified (`is_verified = false`)
- Pending OTP: Available in database
- Test OTP verification flow

### 3. Project Management Test
- Use `proj-001` (Network Programming)
- Owner: john_doe (can modify everything)
- Admin: jane_smith (can manage tasks)
- Member: mike_wilson (can work on tasks)
- Viewer: david_brown (read-only)

### 4. Task Workflow Test
- **Create**: Add new task to project
- **Assign**: Assign to project member
- **Update**: Change status (todo → in_progress → review → completed)
- **Comment**: Add comments to tasks
- **Attach**: Upload files to tasks

### 5. Chat Feature Test
- Join project chat (proj-001)
- View chat history (27 existing messages)
- Send new messages
- Test real-time updates

### 6. Friend System Test
- Send friend request (user-001 → user-005: pending)
- Accept friend request
- Block user (user-005 blocked user-007)
- View friend list

### 7. Notification Test
- Task assignment notifications
- Comment notifications
- Project invite notifications
- Mark as read/unread

## Data Relationships Diagram

```
users (8)
├── projects (6) [as owner]
├── project_members (16) [participation]
├── tasks (20) [assigned & created]
├── task_comments (15) [authored]
├── project_chat_log (27) [messages]
├── file_attachments (14) [uploaded]
├── notifications (19) [received]
├── user_contacts (14) [friendships]
└── otp_verifications (6) [auth codes]
```

## Tips for Testing

1. **Start with proj-001**: Most complete project with all features
2. **Use john_doe**: Has access to multiple projects and roles
3. **Test different roles**: Owner, admin, member, viewer permissions
4. **Check constraints**: Try creating duplicate emails, invalid relations
5. **Test cascading deletes**: Delete a project and see tasks removed
6. **Query performance**: Use EXPLAIN ANALYZE to check indexes
7. **Date ranges**: Adjust dates in queries to test filtering

## Resetting Sample Data

To reload sample data (clears and reimports):
```bash
cd Documents
./load_sample_data.sh
```

Or manually:
```sql
psql -U netprog_user -d netprog_db -f sample_data.sql
```

## Adding Your Own Data

Use the sample data as a template:
1. Copy the INSERT statement format
2. Generate UUIDs for IDs (or use simple strings like 'user-009')
3. Maintain referential integrity (foreign keys)
4. Follow the same data patterns

Example:
```sql
INSERT INTO users (user_id, username, email, password_hash, is_verified) VALUES
('user-009', 'new_user', 'new@example.com', '$2a$10$...', true);

INSERT INTO projects (project_id, project_name, owner_id, status) VALUES
('proj-007', 'My New Project', 'user-009', 'planning');
```

## Support

For issues with sample data:
1. Check database connection: `psql -U netprog_user -d netprog_db`
2. Verify schema exists: `\dn` (should show netprog)
3. Check for errors in script output
4. Review foreign key constraints if inserts fail

Happy testing! 🚀
