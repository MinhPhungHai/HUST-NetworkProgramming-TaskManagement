-- Create schema
CREATE SCHEMA IF NOT EXISTS netprog;

-- Set search path to use the netprog schema
SET search_path TO netprog;

-- Drop tables if they exist (for clean reinstallation)
DROP TABLE IF EXISTS file_attachments CASCADE;
DROP TABLE IF EXISTS task_comments CASCADE;
DROP TABLE IF EXISTS project_chat_log CASCADE;
DROP TABLE IF EXISTS tasks CASCADE;
DROP TABLE IF EXISTS project_members CASCADE;
DROP TABLE IF EXISTS projects CASCADE;
DROP TABLE IF EXISTS user_contacts CASCADE;
DROP TABLE IF EXISTS otp_verifications CASCADE;
DROP TABLE IF EXISTS notifications CASCADE;
DROP TABLE IF EXISTS users CASCADE;

-- Users table
CREATE TABLE users (
    user_id VARCHAR(36) PRIMARY KEY,
    username VARCHAR(255) NOT NULL UNIQUE,
    email VARCHAR(255) NOT NULL UNIQUE,
    phone_number VARCHAR(20) UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    is_verified BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- OTP verifications table (for email/phone verification)
CREATE TABLE otp_verifications (
    otp_id VARCHAR(36) PRIMARY KEY,
    user_id VARCHAR(36) REFERENCES users(user_id) ON DELETE CASCADE,
    email VARCHAR(255),
    otp_code VARCHAR(10) NOT NULL,
    otp_type VARCHAR(20) NOT NULL, -- 'registration', 'login', 'password_reset'
    expires_at TIMESTAMP NOT NULL,
    is_used BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- User contacts/friends table
CREATE TABLE user_contacts (
    contact_id VARCHAR(36) PRIMARY KEY,
    user_id VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    contact_user_id VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    status VARCHAR(20) DEFAULT 'pending', -- 'pending', 'accepted', 'blocked'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT unique_contact UNIQUE(user_id, contact_user_id),
    CONSTRAINT no_self_contact CHECK (user_id != contact_user_id)
);

-- Projects table
CREATE TABLE projects (
    project_id VARCHAR(36) PRIMARY KEY,
    project_name VARCHAR(255) NOT NULL,
    description TEXT,
    owner_id VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    start_date DATE,
    end_date DATE,
    status VARCHAR(20) DEFAULT 'planning', -- 'planning', 'in_progress', 'completed', 'on_hold'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Project members table
CREATE TABLE project_members (
    member_id VARCHAR(36) PRIMARY KEY,
    project_id VARCHAR(36) NOT NULL REFERENCES projects(project_id) ON DELETE CASCADE,
    user_id VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    role VARCHAR(20) DEFAULT 'member', -- 'owner', 'admin', 'member', 'viewer'
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT unique_project_member UNIQUE(project_id, user_id)
);

-- Tasks table
CREATE TABLE tasks (
    task_id VARCHAR(36) PRIMARY KEY,
    project_id VARCHAR(36) NOT NULL REFERENCES projects(project_id) ON DELETE CASCADE,
    task_name VARCHAR(255) NOT NULL,
    description TEXT,
    assigned_to VARCHAR(36) REFERENCES users(user_id) ON DELETE SET NULL,
    created_by VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    status VARCHAR(20) DEFAULT 'todo', -- 'todo', 'in_progress', 'review', 'completed', 'blocked'
    priority VARCHAR(20) DEFAULT 'medium', -- 'low', 'medium', 'high', 'urgent'
    start_date DATE,
    due_date DATE,
    completed_at TIMESTAMP NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Task comments table
CREATE TABLE task_comments (
    comment_id VARCHAR(36) PRIMARY KEY,
    task_id VARCHAR(36) NOT NULL REFERENCES tasks(task_id) ON DELETE CASCADE,
    user_id VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    comment_content TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Project chat log table
CREATE TABLE project_chat_log (
    chat_id VARCHAR(36) PRIMARY KEY,
    project_id VARCHAR(36) NOT NULL REFERENCES projects(project_id) ON DELETE CASCADE,
    sender_id VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    message_content TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- File attachments table
CREATE TABLE file_attachments (
    file_id VARCHAR(36) PRIMARY KEY,
    task_id VARCHAR(36) NOT NULL REFERENCES tasks(task_id) ON DELETE CASCADE,
    file_name VARCHAR(255) NOT NULL,
    file_path VARCHAR(500) NOT NULL,
    file_size BIGINT NOT NULL,
    file_type VARCHAR(100),
    uploaded_by VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Notifications table (for user notifications)
CREATE TABLE notifications (
    notification_id VARCHAR(36) PRIMARY KEY,
    user_id VARCHAR(36) NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    notification_type VARCHAR(50) NOT NULL, -- 'task_assigned', 'project_invite', 'comment_added', 'task_updated', etc.
    related_entity_type VARCHAR(50), -- 'task', 'project', 'comment', etc.
    related_entity_id VARCHAR(36),
    message TEXT NOT NULL,
    is_read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create indexes for better query performance
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_is_verified ON users(is_verified);

CREATE INDEX idx_otp_email ON otp_verifications(email);
CREATE INDEX idx_otp_user_id ON otp_verifications(user_id);
CREATE INDEX idx_otp_expires_at ON otp_verifications(expires_at);

CREATE INDEX idx_user_contacts_user_id ON user_contacts(user_id);
CREATE INDEX idx_user_contacts_contact_user_id ON user_contacts(contact_user_id);
CREATE INDEX idx_user_contacts_status ON user_contacts(status);

CREATE INDEX idx_projects_owner_id ON projects(owner_id);
CREATE INDEX idx_projects_status ON projects(status);

CREATE INDEX idx_project_members_project_id ON project_members(project_id);
CREATE INDEX idx_project_members_user_id ON project_members(user_id);

CREATE INDEX idx_tasks_project_id ON tasks(project_id);
CREATE INDEX idx_tasks_assigned_to ON tasks(assigned_to);
CREATE INDEX idx_tasks_created_by ON tasks(created_by);
CREATE INDEX idx_tasks_status ON tasks(status);
CREATE INDEX idx_tasks_due_date ON tasks(due_date);

CREATE INDEX idx_task_comments_task_id ON task_comments(task_id);
CREATE INDEX idx_task_comments_user_id ON task_comments(user_id);

CREATE INDEX idx_project_chat_log_project_id ON project_chat_log(project_id);
CREATE INDEX idx_project_chat_log_sender_id ON project_chat_log(sender_id);
CREATE INDEX idx_project_chat_log_created_at ON project_chat_log(created_at);

CREATE INDEX idx_file_attachments_task_id ON file_attachments(task_id);
CREATE INDEX idx_file_attachments_uploaded_by ON file_attachments(uploaded_by);

CREATE INDEX idx_notifications_user_id ON notifications(user_id);
CREATE INDEX idx_notifications_is_read ON notifications(is_read);
CREATE INDEX idx_notifications_created_at ON notifications(created_at);

-- Create function to update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Create triggers for updated_at
CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_projects_updated_at BEFORE UPDATE ON projects
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_tasks_updated_at BEFORE UPDATE ON tasks
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_task_comments_updated_at BEFORE UPDATE ON task_comments
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Grant permissions (adjust as needed)
GRANT ALL PRIVILEGES ON SCHEMA netprog TO CURRENT_USER;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA netprog TO CURRENT_USER;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA netprog TO CURRENT_USER;

-- Insert sample data (optional - comment out if not needed)
-- You can uncomment these to test the schema
/*
INSERT INTO users (user_id, username, email, password_hash, is_verified) VALUES
('user-001', 'john_doe', 'john@example.com', '$2a$10$abcdefghijklmnopqrstuvwxyz', true),
('user-002', 'jane_smith', 'jane@example.com', '$2a$10$abcdefghijklmnopqrstuvwxyz', true);

INSERT INTO projects (project_id, project_name, description, owner_id, status) VALUES
('proj-001', 'Sample Project', 'This is a sample project', 'user-001', 'in_progress');

INSERT INTO project_members (member_id, project_id, user_id, role) VALUES
('mem-001', 'proj-001', 'user-001', 'owner'),
('mem-002', 'proj-001', 'user-002', 'member');
*/

COMMENT ON SCHEMA netprog IS 'Network Programming Task Management System';
COMMENT ON TABLE users IS 'Stores user account information';
COMMENT ON TABLE otp_verifications IS 'Stores OTP codes for email/phone verification';
COMMENT ON TABLE user_contacts IS 'Stores friend/contact relationships between users';
COMMENT ON TABLE projects IS 'Stores project information';
COMMENT ON TABLE project_members IS 'Stores project membership and roles';
COMMENT ON TABLE tasks IS 'Stores tasks within projects';
COMMENT ON TABLE task_comments IS 'Stores comments on tasks';
COMMENT ON TABLE project_chat_log IS 'Stores chat messages within projects';
COMMENT ON TABLE file_attachments IS 'Stores file attachments for tasks';
COMMENT ON TABLE notifications IS 'Stores user notifications';
