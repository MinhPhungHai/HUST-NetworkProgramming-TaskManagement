-- Sample Data for Network Programming Task Management System
-- This file contains example data for testing and demonstration purposes

-- Set search path to netprog schema
SET search_path TO netprog;

-- Clear existing data (if any)
TRUNCATE TABLE file_attachments, task_comments, project_chat_log, tasks, project_members, projects, user_contacts, otp_verifications, notifications, users CASCADE;

-- ============================================
-- 1. USERS DATA
-- ============================================
-- Note: Password is hashed version of "password123" using bcrypt
-- In production, use proper password hashing libraries

INSERT INTO users (user_id, username, email, phone_number, password_hash, is_verified, created_at) VALUES
('user-001', 'john_doe', 'john.doe@example.com', '+84901234567', '$2a$10$rZ5QhbKd5J5r5J5r5J5r5OqK5J5r5J5r5J5r5J5r5J5r5J5r5J5r5', true, '2025-01-01 10:00:00'),
('user-002', 'jane_smith', 'jane.smith@example.com', '+84901234568', '$2a$10$rZ5QhbKd5J5r5J5r5J5r5OqK5J5r5J5r5J5r5J5r5J5r5J5r5J5r5', true, '2025-01-01 11:00:00'),
('user-003', 'mike_wilson', 'mike.wilson@example.com', '+84901234569', '$2a$10$rZ5QhbKd5J5r5J5r5J5r5OqK5J5r5J5r5J5r5J5r5J5r5J5r5J5r5', true, '2025-01-02 09:00:00'),
('user-004', 'sarah_johnson', 'sarah.johnson@example.com', '+84901234570', '$2a$10$rZ5QhbKd5J5r5J5r5J5r5OqK5J5r5J5r5J5r5J5r5J5r5J5r5J5r5', true, '2025-01-02 10:30:00'),
('user-005', 'david_brown', 'david.brown@example.com', '+84901234571', '$2a$10$rZ5QhbKd5J5r5J5r5J5r5OqK5J5r5J5r5J5r5J5r5J5r5J5r5J5r5', true, '2025-01-03 08:00:00'),
('user-006', 'emily_davis', 'emily.davis@example.com', '+84901234572', '$2a$10$rZ5QhbKd5J5r5J5r5J5r5OqK5J5r5J5r5J5r5J5r5J5r5J5r5J5r5', false, '2025-01-04 14:00:00'),
('user-007', 'alex_nguyen', 'alex.nguyen@example.com', '+84901234573', '$2a$10$rZ5QhbKd5J5r5J5r5J5r5OqK5J5r5J5r5J5r5J5r5J5r5J5r5J5r5', true, '2025-01-04 15:30:00'),
('user-008', 'lisa_tran', 'lisa.tran@example.com', '+84901234574', '$2a$10$rZ5QhbKd5J5r5J5r5J5r5OqK5J5r5J5r5J5r5J5r5J5r5J5r5J5r5', true, '2025-01-05 09:00:00');

-- ============================================
-- 2. OTP VERIFICATIONS DATA
-- ============================================
INSERT INTO otp_verifications (otp_id, user_id, email, otp_code, otp_type, expires_at, is_used, created_at) VALUES
-- Used OTPs (for login history)
('otp-001', 'user-001', 'john.doe@example.com', '123456', 'login', '2025-01-06 10:15:00', true, '2025-01-06 10:00:00'),
('otp-002', 'user-002', 'jane.smith@example.com', '234567', 'login', '2025-01-06 11:15:00', true, '2025-01-06 11:00:00'),
('otp-003', 'user-001', 'john.doe@example.com', '345678', 'registration', '2025-01-01 10:15:00', true, '2025-01-01 10:00:00'),
-- Pending OTP for new user
('otp-004', 'user-006', 'emily.davis@example.com', '456789', 'registration', '2025-01-07 14:15:00', false, '2025-01-07 14:00:00'),
-- Recent login OTP
('otp-005', 'user-003', 'mike.wilson@example.com', '567890', 'login', '2025-01-07 09:15:00', true, '2025-01-07 09:00:00'),
-- Password reset OTP
('otp-006', 'user-004', 'sarah.johnson@example.com', '678901', 'password_reset', '2025-01-07 11:15:00', false, '2025-01-07 11:00:00');

-- ============================================
-- 3. USER CONTACTS (Friend System)
-- ============================================
INSERT INTO user_contacts (contact_id, user_id, contact_user_id, status, created_at) VALUES
-- John's friends
('contact-001', 'user-001', 'user-002', 'accepted', '2025-01-02 10:00:00'),
('contact-002', 'user-001', 'user-003', 'accepted', '2025-01-02 11:00:00'),
('contact-003', 'user-001', 'user-004', 'accepted', '2025-01-02 12:00:00'),
('contact-004', 'user-001', 'user-005', 'pending', '2025-01-06 10:00:00'),
-- Jane's friends
('contact-005', 'user-002', 'user-001', 'accepted', '2025-01-02 10:00:00'),
('contact-006', 'user-002', 'user-003', 'accepted', '2025-01-03 09:00:00'),
('contact-007', 'user-002', 'user-007', 'accepted', '2025-01-05 10:00:00'),
-- Mike's friends
('contact-008', 'user-003', 'user-001', 'accepted', '2025-01-02 11:00:00'),
('contact-009', 'user-003', 'user-002', 'accepted', '2025-01-03 09:00:00'),
('contact-010', 'user-003', 'user-008', 'pending', '2025-01-06 14:00:00'),
-- Sarah's friends
('contact-011', 'user-004', 'user-001', 'accepted', '2025-01-02 12:00:00'),
('contact-012', 'user-004', 'user-005', 'accepted', '2025-01-04 10:00:00'),
-- David's friends
('contact-013', 'user-005', 'user-004', 'accepted', '2025-01-04 10:00:00'),
('contact-014', 'user-005', 'user-007', 'blocked', '2025-01-05 15:00:00');

-- ============================================
-- 4. PROJECTS DATA
-- ============================================
INSERT INTO projects (project_id, project_name, description, owner_id, start_date, end_date, status, created_at) VALUES
-- Network Programming Final Project (main project)
('proj-001', 'Network Programming Final Project', 'Task Management System with client-server architecture using socket programming', 'user-001', '2025-01-01', '2025-02-28', 'in_progress', '2025-01-01 10:30:00'),
-- Web Development Project
('proj-002', 'E-Commerce Website', 'Full-stack e-commerce platform with React and Node.js', 'user-002', '2025-01-05', '2025-03-15', 'in_progress', '2025-01-05 11:00:00'),
-- Mobile App Project
('proj-003', 'Fitness Tracking App', 'Mobile application for tracking workouts and nutrition', 'user-003', '2025-01-10', '2025-04-10', 'planning', '2025-01-06 09:00:00'),
-- Database Project
('proj-004', 'Database Migration Tool', 'Tool for migrating data between different database systems', 'user-001', '2024-12-01', '2025-01-15', 'completed', '2024-12-01 08:00:00'),
-- Machine Learning Project
('proj-005', 'Image Recognition System', 'CNN-based image classification system', 'user-004', '2025-01-15', '2025-05-01', 'planning', '2025-01-06 14:00:00'),
-- Personal Project
('proj-006', 'Personal Portfolio Website', 'Portfolio website showcasing projects and skills', 'user-005', '2025-01-01', '2025-01-20', 'on_hold', '2025-01-03 10:00:00');

-- ============================================
-- 5. PROJECT MEMBERS DATA
-- ============================================
INSERT INTO project_members (member_id, project_id, user_id, role, joined_at) VALUES
-- Network Programming Project Team (proj-001)
('member-001', 'proj-001', 'user-001', 'owner', '2025-01-01 10:30:00'),
('member-002', 'proj-001', 'user-002', 'admin', '2025-01-01 11:00:00'),
('member-003', 'proj-001', 'user-003', 'member', '2025-01-02 09:00:00'),
('member-004', 'proj-001', 'user-004', 'member', '2025-01-02 10:00:00'),
('member-005', 'proj-001', 'user-005', 'viewer', '2025-01-03 08:00:00'),
-- E-Commerce Project Team (proj-002)
('member-006', 'proj-002', 'user-002', 'owner', '2025-01-05 11:00:00'),
('member-007', 'proj-002', 'user-001', 'member', '2025-01-05 12:00:00'),
('member-008', 'proj-002', 'user-007', 'member', '2025-01-05 13:00:00'),
-- Fitness App Team (proj-003)
('member-009', 'proj-003', 'user-003', 'owner', '2025-01-06 09:00:00'),
('member-010', 'proj-003', 'user-008', 'member', '2025-01-06 10:00:00'),
('member-011', 'proj-003', 'user-002', 'admin', '2025-01-06 11:00:00'),
-- Database Migration Tool (proj-004)
('member-012', 'proj-004', 'user-001', 'owner', '2024-12-01 08:00:00'),
('member-013', 'proj-004', 'user-005', 'member', '2024-12-01 09:00:00'),
-- Image Recognition Team (proj-005)
('member-014', 'proj-005', 'user-004', 'owner', '2025-01-06 14:00:00'),
('member-015', 'proj-005', 'user-001', 'member', '2025-01-06 15:00:00'),
-- Personal Portfolio (proj-006)
('member-016', 'proj-006', 'user-005', 'owner', '2025-01-03 10:00:00');

-- ============================================
-- 6. TASKS DATA
-- ============================================
INSERT INTO tasks (task_id, project_id, task_name, description, assigned_to, created_by, status, priority, start_date, due_date, completed_at, created_at) VALUES
-- Network Programming Project Tasks (proj-001)
('task-001', 'proj-001', 'Design Database Schema', 'Create comprehensive database schema with all required tables and relationships', 'user-001', 'user-001', 'completed', 'high', '2025-01-01', '2025-01-03', '2025-01-03 16:00:00', '2025-01-01 11:00:00'),
('task-002', 'proj-001', 'Implement Server Architecture', 'Design and implement TCP server with multi-threading support', 'user-002', 'user-001', 'in_progress', 'high', '2025-01-03', '2025-01-15', NULL, '2025-01-01 11:30:00'),
('task-003', 'proj-001', 'Create Client Application', 'Build client application with GUI for task management', 'user-003', 'user-001', 'in_progress', 'high', '2025-01-05', '2025-01-20', NULL, '2025-01-01 12:00:00'),
('task-004', 'proj-001', 'Implement OTP Verification', 'Add OTP email verification for login and registration', 'user-004', 'user-002', 'todo', 'medium', '2025-01-10', '2025-01-18', NULL, '2025-01-02 09:00:00'),
('task-005', 'proj-001', 'Design Protocol Messages', 'Define message structure and protocol for client-server communication', 'user-002', 'user-001', 'review', 'high', '2025-01-02', '2025-01-06', NULL, '2025-01-01 13:00:00'),
('task-006', 'proj-001', 'Implement Chat Feature', 'Add real-time chat functionality within projects', 'user-003', 'user-002', 'todo', 'medium', '2025-01-12', '2025-01-22', NULL, '2025-01-02 10:00:00'),
('task-007', 'proj-001', 'Create Gantt Chart View', 'Implement Gantt chart visualization for project timeline', 'user-004', 'user-001', 'blocked', 'low', '2025-01-15', '2025-01-25', NULL, '2025-01-02 11:00:00'),
('task-008', 'proj-001', 'Write Unit Tests', 'Create comprehensive unit tests for all components', 'user-005', 'user-002', 'todo', 'medium', '2025-01-18', '2025-02-05', NULL, '2025-01-02 12:00:00'),
('task-009', 'proj-001', 'Documentation', 'Write technical documentation and user manual', 'user-001', 'user-001', 'todo', 'low', '2025-02-01', '2025-02-20', NULL, '2025-01-02 13:00:00'),
-- E-Commerce Project Tasks (proj-002)
('task-010', 'proj-002', 'Setup Project Structure', 'Initialize React and Node.js project structure', 'user-002', 'user-002', 'completed', 'high', '2025-01-05', '2025-01-06', '2025-01-06 17:00:00', '2025-01-05 11:30:00'),
('task-011', 'proj-002', 'Design Product Catalog', 'Create product listing and detail pages', 'user-007', 'user-002', 'in_progress', 'high', '2025-01-06', '2025-01-15', NULL, '2025-01-05 12:00:00'),
('task-012', 'proj-002', 'Implement Shopping Cart', 'Build shopping cart functionality with persistence', 'user-001', 'user-002', 'todo', 'high', '2025-01-12', '2025-01-20', NULL, '2025-01-05 13:00:00'),
('task-013', 'proj-002', 'Payment Integration', 'Integrate payment gateway (Stripe/PayPal)', 'user-002', 'user-002', 'todo', 'urgent', '2025-01-18', '2025-01-28', NULL, '2025-01-05 14:00:00'),
-- Fitness App Tasks (proj-003)
('task-014', 'proj-003', 'UI/UX Design', 'Create wireframes and mockups for mobile app', 'user-008', 'user-003', 'in_progress', 'high', '2025-01-06', '2025-01-12', NULL, '2025-01-06 09:30:00'),
('task-015', 'proj-003', 'Setup React Native', 'Initialize React Native project with navigation', 'user-003', 'user-003', 'todo', 'high', '2025-01-10', '2025-01-13', NULL, '2025-01-06 10:00:00'),
-- Database Migration Tool Tasks (proj-004) - Completed project
('task-016', 'proj-004', 'Support MySQL Migration', 'Implement MySQL to PostgreSQL migration', 'user-001', 'user-001', 'completed', 'high', '2024-12-01', '2024-12-15', '2024-12-15 18:00:00', '2024-12-01 08:30:00'),
('task-017', 'proj-004', 'Add Data Validation', 'Implement data validation and error handling', 'user-005', 'user-001', 'completed', 'medium', '2024-12-10', '2024-12-20', '2024-12-20 16:00:00', '2024-12-01 09:00:00'),
('task-018', 'proj-004', 'Performance Testing', 'Test with large datasets and optimize', 'user-001', 'user-001', 'completed', 'medium', '2024-12-15', '2025-01-10', '2025-01-10 15:00:00', '2024-12-01 10:00:00'),
-- Image Recognition Tasks (proj-005)
('task-019', 'proj-005', 'Research CNN Architectures', 'Study and compare different CNN models', 'user-004', 'user-004', 'in_progress', 'high', '2025-01-06', '2025-01-12', NULL, '2025-01-06 14:30:00'),
('task-020', 'proj-005', 'Collect Training Data', 'Gather and label training dataset', 'user-001', 'user-004', 'todo', 'high', '2025-01-08', '2025-01-18', NULL, '2025-01-06 15:00:00');

-- ============================================
-- 7. TASK COMMENTS DATA
-- ============================================
INSERT INTO task_comments (comment_id, task_id, user_id, comment_content, created_at) VALUES
-- Comments on Database Schema task
('comment-001', 'task-001', 'user-002', 'Great work on the schema! I suggest adding indexes for better query performance.', '2025-01-02 10:00:00'),
('comment-002', 'task-001', 'user-001', 'Good idea! I will add indexes on all foreign keys and frequently queried columns.', '2025-01-02 10:30:00'),
('comment-003', 'task-001', 'user-003', 'Should we add a notifications table for user alerts?', '2025-01-02 11:00:00'),
('comment-004', 'task-001', 'user-001', 'Yes, added notifications table to the schema.', '2025-01-02 12:00:00'),
-- Comments on Server Architecture task
('comment-005', 'task-002', 'user-001', 'Make sure to implement proper connection pooling for handling multiple clients.', '2025-01-04 09:00:00'),
('comment-006', 'task-002', 'user-002', 'Working on thread pool implementation. Will have it ready by tomorrow.', '2025-01-04 14:00:00'),
('comment-007', 'task-002', 'user-003', 'Need to coordinate with client implementation. Can we have a meeting?', '2025-01-05 10:00:00'),
-- Comments on Protocol Design task
('comment-008', 'task-005', 'user-002', 'I have drafted the protocol specification. Please review the message formats.', '2025-01-05 11:00:00'),
('comment-009', 'task-005', 'user-001', 'Looks good! We should add error codes for different failure scenarios.', '2025-01-05 15:00:00'),
('comment-010', 'task-005', 'user-004', 'Should we use JSON or binary format for messages?', '2025-01-06 09:00:00'),
('comment-011', 'task-005', 'user-002', 'JSON would be easier to debug. Binary can be added later for optimization.', '2025-01-06 10:00:00'),
-- Comments on Gantt Chart task
('comment-012', 'task-007', 'user-004', 'Blocked: Need to wait for task data structure to be finalized first.', '2025-01-06 14:00:00'),
('comment-013', 'task-007', 'user-001', 'The task structure is ready now. You can proceed with implementation.', '2025-01-06 15:00:00'),
-- Comments on E-Commerce tasks
('comment-014', 'task-011', 'user-007', 'Working on product grid layout. Should have mockup ready soon.', '2025-01-07 09:00:00'),
('comment-015', 'task-013', 'user-002', 'We need to decide between Stripe and PayPal. Stripe has better API documentation.', '2025-01-06 10:00:00');

-- ============================================
-- 8. PROJECT CHAT LOG DATA
-- ============================================
INSERT INTO project_chat_log (chat_id, project_id, sender_id, message_content, created_at) VALUES
-- Network Programming Project Chat (proj-001)
('chat-001', 'proj-001', 'user-001', 'Welcome everyone to the Network Programming project! Let us coordinate our efforts here.', '2025-01-01 11:00:00'),
('chat-002', 'proj-001', 'user-002', 'Thanks for creating this project! Excited to work on the server side.', '2025-01-01 11:30:00'),
('chat-003', 'proj-001', 'user-003', 'I will focus on the client application and UI design.', '2025-01-02 09:00:00'),
('chat-004', 'proj-001', 'user-004', 'Happy to help with OTP verification and security features.', '2025-01-02 10:00:00'),
('chat-005', 'proj-001', 'user-001', 'Great! I have assigned initial tasks to everyone. Please check your task list.', '2025-01-02 11:00:00'),
('chat-006', 'proj-001', 'user-002', 'The database schema looks comprehensive. Nice work @john_doe!', '2025-01-03 10:00:00'),
('chat-007', 'proj-001', 'user-003', 'When can we have our first team meeting to discuss the protocol?', '2025-01-03 14:00:00'),
('chat-008', 'proj-001', 'user-001', 'How about tomorrow at 2 PM? I will send a calendar invite.', '2025-01-03 15:00:00'),
('chat-009', 'proj-001', 'user-002', 'Sounds good! I will prepare the protocol draft before the meeting.', '2025-01-03 16:00:00'),
('chat-010', 'proj-001', 'user-004', 'Quick question: Should OTP be 6 digits or 8 digits?', '2025-01-04 09:00:00'),
('chat-011', 'proj-001', 'user-001', '6 digits should be sufficient. Standard practice for OTP.', '2025-01-04 09:30:00'),
('chat-012', 'proj-001', 'user-005', 'I am reviewing the code. Will provide feedback soon.', '2025-01-04 10:00:00'),
('chat-013', 'proj-001', 'user-003', 'Made good progress on the client UI today. Will share screenshots tomorrow.', '2025-01-05 17:00:00'),
('chat-014', 'proj-001', 'user-002', 'Server is handling multiple connections now! Thread pool is working great.', '2025-01-06 15:00:00'),
('chat-015', 'proj-001', 'user-001', 'Excellent progress everyone! Keep up the good work!', '2025-01-06 16:00:00'),
-- E-Commerce Project Chat (proj-002)
('chat-016', 'proj-002', 'user-002', 'Starting the e-commerce platform. This is going to be exciting!', '2025-01-05 11:30:00'),
('chat-017', 'proj-002', 'user-001', 'Count me in! I can help with backend APIs.', '2025-01-05 12:00:00'),
('chat-018', 'proj-002', 'user-007', 'I will work on the frontend and product catalog.', '2025-01-05 13:00:00'),
('chat-019', 'proj-002', 'user-002', 'Perfect! Let us use MongoDB for the product database.', '2025-01-05 14:00:00'),
('chat-020', 'proj-002', 'user-001', 'Agreed. I will set up the Node.js server with Express.', '2025-01-06 09:00:00'),
-- Fitness App Chat (proj-003)
('chat-021', 'proj-003', 'user-003', 'Fitness tracking app project starts now! Who wants to join?', '2025-01-06 09:00:00'),
('chat-022', 'proj-003', 'user-008', 'I am in! I can design the UI/UX.', '2025-01-06 10:00:00'),
('chat-023', 'proj-003', 'user-002', 'I have experience with React Native. Happy to help!', '2025-01-06 11:00:00'),
('chat-024', 'proj-003', 'user-003', 'Awesome team! Let us make this app amazing.', '2025-01-06 12:00:00'),
-- Image Recognition Chat (proj-005)
('chat-025', 'proj-005', 'user-004', 'Starting the image recognition project. Anyone interested in ML?', '2025-01-06 14:00:00'),
('chat-026', 'proj-005', 'user-001', 'I am interested! I have worked with TensorFlow before.', '2025-01-06 15:00:00'),
('chat-027', 'proj-005', 'user-004', 'Great! Let us research the best CNN architecture first.', '2025-01-06 16:00:00');

-- ============================================
-- 9. FILE ATTACHMENTS DATA
-- ============================================
INSERT INTO file_attachments (file_id, task_id, file_name, file_path, file_size, file_type, uploaded_by, created_at) VALUES
-- Files for Database Schema task
('file-001', 'task-001', 'database_schema.sql', '/uploads/proj-001/task-001/database_schema.sql', 45678, 'application/sql', 'user-001', '2025-01-02 14:00:00'),
('file-002', 'task-001', 'schema_diagram.png', '/uploads/proj-001/task-001/schema_diagram.png', 234567, 'image/png', 'user-001', '2025-01-02 15:00:00'),
('file-003', 'task-001', 'database_design.pdf', '/uploads/proj-001/task-001/database_design.pdf', 1234567, 'application/pdf', 'user-002', '2025-01-03 10:00:00'),
-- Files for Server Architecture task
('file-004', 'task-002', 'server_architecture.drawio', '/uploads/proj-001/task-002/server_architecture.drawio', 89012, 'application/xml', 'user-002', '2025-01-04 11:00:00'),
('file-005', 'task-002', 'server_code.zip', '/uploads/proj-001/task-002/server_code.zip', 3456789, 'application/zip', 'user-002', '2025-01-05 16:00:00'),
-- Files for Client Application task
('file-006', 'task-003', 'ui_mockup.fig', '/uploads/proj-001/task-003/ui_mockup.fig', 5678901, 'application/octet-stream', 'user-003', '2025-01-05 14:00:00'),
('file-007', 'task-003', 'client_screenshot.png', '/uploads/proj-001/task-003/client_screenshot.png', 456789, 'image/png', 'user-003', '2025-01-06 10:00:00'),
-- Files for Protocol Design task
('file-008', 'task-005', 'protocol_specification.docx', '/uploads/proj-001/task-005/protocol_specification.docx', 234567, 'application/vnd.openxmlformats-officedocument.wordprocessingml.document', 'user-002', '2025-01-05 12:00:00'),
('file-009', 'task-005', 'message_format.json', '/uploads/proj-001/task-005/message_format.json', 12345, 'application/json', 'user-002', '2025-01-05 13:00:00'),
-- Files for E-Commerce tasks
('file-010', 'task-011', 'product_catalog_design.psd', '/uploads/proj-002/task-011/product_catalog_design.psd', 12345678, 'application/octet-stream', 'user-007', '2025-01-07 10:00:00'),
('file-011', 'task-013', 'payment_flow_diagram.pdf', '/uploads/proj-002/task-013/payment_flow_diagram.pdf', 890123, 'application/pdf', 'user-002', '2025-01-06 11:00:00'),
-- Files for Fitness App tasks
('file-012', 'task-014', 'wireframes.sketch', '/uploads/proj-003/task-014/wireframes.sketch', 6789012, 'application/octet-stream', 'user-008', '2025-01-06 15:00:00'),
-- Files for completed project
('file-013', 'task-016', 'migration_script.py', '/uploads/proj-004/task-016/migration_script.py', 45678, 'text/x-python', 'user-001', '2024-12-10 14:00:00'),
('file-014', 'task-018', 'performance_results.xlsx', '/uploads/proj-004/task-018/performance_results.xlsx', 234567, 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet', 'user-001', '2025-01-08 11:00:00');

-- ============================================
-- 10. NOTIFICATIONS DATA
-- ============================================
INSERT INTO notifications (notification_id, user_id, notification_type, related_entity_type, related_entity_id, message, is_read, created_at) VALUES
-- Notifications for John (user-001)
('notif-001', 'user-001', 'task_assigned', 'task', 'task-001', 'You have been assigned to task: Design Database Schema', true, '2025-01-01 11:00:00'),
('notif-002', 'user-001', 'comment_added', 'task', 'task-001', 'Jane Smith commented on task: Design Database Schema', true, '2025-01-02 10:00:00'),
('notif-003', 'user-001', 'project_invite', 'project', 'proj-002', 'Jane Smith invited you to join project: E-Commerce Website', true, '2025-01-05 12:00:00'),
('notif-004', 'user-001', 'task_updated', 'task', 'task-002', 'Server Architecture task status changed to in_progress', false, '2025-01-06 09:00:00'),
-- Notifications for Jane (user-002)
('notif-005', 'user-002', 'task_assigned', 'task', 'task-002', 'You have been assigned to task: Implement Server Architecture', true, '2025-01-01 11:30:00'),
('notif-006', 'user-002', 'project_invite', 'project', 'proj-001', 'John Doe invited you to join project: Network Programming Final Project', true, '2025-01-01 11:00:00'),
('notif-007', 'user-002', 'comment_added', 'task', 'task-005', 'Sarah Johnson commented on task: Design Protocol Messages', false, '2025-01-06 09:00:00'),
-- Notifications for Mike (user-003)
('notif-008', 'user-003', 'task_assigned', 'task', 'task-003', 'You have been assigned to task: Create Client Application', true, '2025-01-01 12:00:00'),
('notif-009', 'user-003', 'task_assigned', 'task', 'task-006', 'You have been assigned to task: Implement Chat Feature', false, '2025-01-02 10:00:00'),
('notif-010', 'user-003', 'project_invite', 'project', 'proj-003', 'You created a new project: Fitness Tracking App', true, '2025-01-06 09:00:00'),
-- Notifications for Sarah (user-004)
('notif-011', 'user-004', 'task_assigned', 'task', 'task-004', 'You have been assigned to task: Implement OTP Verification', false, '2025-01-02 09:00:00'),
('notif-012', 'user-004', 'task_assigned', 'task', 'task-007', 'You have been assigned to task: Create Gantt Chart View', false, '2025-01-02 11:00:00'),
('notif-013', 'user-004', 'comment_added', 'task', 'task-001', 'Your comment received a reply from John Doe', true, '2025-01-02 12:00:00'),
-- Notifications for David (user-005)
('notif-014', 'user-005', 'task_assigned', 'task', 'task-008', 'You have been assigned to task: Write Unit Tests', false, '2025-01-02 12:00:00'),
('notif-015', 'user-005', 'project_invite', 'project', 'proj-001', 'John Doe invited you to join project: Network Programming Final Project', true, '2025-01-03 08:00:00'),
-- Notifications for Alex (user-007)
('notif-016', 'user-007', 'task_assigned', 'task', 'task-011', 'You have been assigned to task: Design Product Catalog', true, '2025-01-05 12:00:00'),
('notif-017', 'user-007', 'project_invite', 'project', 'proj-002', 'Jane Smith invited you to join project: E-Commerce Website', true, '2025-01-05 13:00:00'),
-- Notifications for Lisa (user-008)
('notif-018', 'user-008', 'task_assigned', 'task', 'task-014', 'You have been assigned to task: UI/UX Design', false, '2025-01-06 09:30:00'),
('notif-019', 'user-008', 'project_invite', 'project', 'proj-003', 'Mike Wilson invited you to join project: Fitness Tracking App', true, '2025-01-06 10:00:00');

-- ============================================
-- VERIFICATION QUERIES
-- ============================================

-- Count all records
DO $$
DECLARE
    user_count INTEGER;
    otp_count INTEGER;
    contact_count INTEGER;
    project_count INTEGER;
    member_count INTEGER;
    task_count INTEGER;
    comment_count INTEGER;
    chat_count INTEGER;
    file_count INTEGER;
    notif_count INTEGER;
BEGIN
    SELECT COUNT(*) INTO user_count FROM users;
    SELECT COUNT(*) INTO otp_count FROM otp_verifications;
    SELECT COUNT(*) INTO contact_count FROM user_contacts;
    SELECT COUNT(*) INTO project_count FROM projects;
    SELECT COUNT(*) INTO member_count FROM project_members;
    SELECT COUNT(*) INTO task_count FROM tasks;
    SELECT COUNT(*) INTO comment_count FROM task_comments;
    SELECT COUNT(*) INTO chat_count FROM project_chat_log;
    SELECT COUNT(*) INTO file_count FROM file_attachments;
    SELECT COUNT(*) INTO notif_count FROM notifications;

    RAISE NOTICE '==========================================';
    RAISE NOTICE 'Sample Data Import Summary';
    RAISE NOTICE '==========================================';
    RAISE NOTICE 'Users: %', user_count;
    RAISE NOTICE 'OTP Verifications: %', otp_count;
    RAISE NOTICE 'User Contacts: %', contact_count;
    RAISE NOTICE 'Projects: %', project_count;
    RAISE NOTICE 'Project Members: %', member_count;
    RAISE NOTICE 'Tasks: %', task_count;
    RAISE NOTICE 'Task Comments: %', comment_count;
    RAISE NOTICE 'Project Chat Messages: %', chat_count;
    RAISE NOTICE 'File Attachments: %', file_count;
    RAISE NOTICE 'Notifications: %', notif_count;
    RAISE NOTICE '==========================================';
    RAISE NOTICE 'Sample data imported successfully!';
    RAISE NOTICE '==========================================';
END $$;

-- Show some sample queries
RAISE NOTICE '';
RAISE NOTICE 'Try these sample queries:';
RAISE NOTICE '1. View all projects: SELECT * FROM projects;';
RAISE NOTICE '2. View tasks with assignees: SELECT t.task_name, u.username AS assigned_to, t.status FROM tasks t LEFT JOIN users u ON t.assigned_to = u.user_id;';
RAISE NOTICE '3. View project chat: SELECT u.username, p.message_content, p.created_at FROM project_chat_log p JOIN users u ON p.sender_id = u.user_id WHERE p.project_id = ''proj-001'' ORDER BY p.created_at;';
RAISE NOTICE '4. View user notifications: SELECT message, is_read, created_at FROM notifications WHERE user_id = ''user-001'' ORDER BY created_at DESC;';
