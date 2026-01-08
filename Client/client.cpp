#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "network_manager.h"
#include "session_manager.h"

using namespace std;

// Global instances
NetworkManager g_network("127.0.0.1", 8080);
SessionManager g_session;

// Clear screen
void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

// Display menu
void displayMenu() {
    cout << "\n========================================" << endl;
    if (g_session.isLoggedIn()) {
        cout << "  Logged in as: " << g_session.getCurrentUsername() << endl;
    } else {
        cout << "  Not logged in" << endl;
    }
    cout << "========================================" << endl;
    cout << "1. Register" << endl;
    cout << "2. Login" << endl;
    cout << "3. Get My Projects" << endl;
    cout << "4. Create Project" << endl;
    cout << "5. Get Tasks (for current project)" << endl;
    cout << "6. Create Task" << endl;
    cout << "7. Update Task Status" << endl;
    cout << "8. Send Chat Message" << endl;
    cout << "9. View Chat History" << endl;
    cout << "10. Add Contact" << endl;
    cout << "11. View Contacts" << endl;
    cout << "12. Logout" << endl;
    cout << "0. Exit" << endl;
    cout << "========================================" << endl;
    cout << "Choose option: ";
}

// Handle registration
void handleRegister() {
    string username, email, password;

    cout << "\n=== REGISTER ===" << endl;
    cout << "Username: ";
    cin >> username;
    cout << "Email: ";
    cin >> email;
    cout << "Password: ";
    cin >> password;

    string response;
    cout << "Sending registration request..." << endl;

    if (!g_network.registerUser(username, email, password, response)) {
        cout << "ERROR: Failed to send registration request!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status == STATUS_USER_ALREADY_EXISTS) {
        cout << "ERROR: Username or email already exists!" << endl;
        return;
    }

    if (status != STATUS_SUCCESS) {
        cout << "ERROR: Registration failed!" << endl;
        return;
    }

    cout << "Registration request sent!" << endl;
    cout << "CHECK SERVER TERMINAL FOR OTP CODE!" << endl;
    cout << "Enter OTP: ";
    string otp;
    cin >> otp;

    if (!g_network.verifyOTP(email, otp, "registration", response)) {
        cout << "ERROR: OTP verification failed!" << endl;
        return;
    }

    parser = JsonParser(response);
    status = parser.getInt("status");

    if (status == STATUS_SUCCESS) {
        cout << "SUCCESS: Registration complete! You can now login." << endl;
    } else if (status == STATUS_INVALID_OTP) {
        cout << "ERROR: Invalid OTP code!" << endl;
    } else if (status == STATUS_OTP_EXPIRED) {
        cout << "ERROR: OTP expired!" << endl;
    } else {
        cout << "ERROR: Registration failed!" << endl;
    }
}

// Handle login
void handleLogin() {
    string username, password;

    cout << "\n=== LOGIN ===" << endl;
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;

    string response;
    cout << "Sending login request..." << endl;

    if (!g_network.login(username, password, response)) {
        cout << "ERROR: Failed to send login request!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status == STATUS_INVALID_CREDENTIALS) {
        cout << "ERROR: Invalid username or password!" << endl;
        return;
    }

    if (status != STATUS_SUCCESS) {
        cout << "ERROR: Login failed!" << endl;
        return;
    }

    string email = parser.getString("email");

    cout << "Login request sent!" << endl;
    cout << "CHECK SERVER TERMINAL FOR OTP CODE!" << endl;
    cout << "Enter OTP: ";
    string otp;
    cin >> otp;

    if (!g_network.verifyOTP(email, otp, "login", response)) {
        cout << "ERROR: OTP verification failed!" << endl;
        return;
    }

    parser = JsonParser(response);
    status = parser.getInt("status");

    if (status == STATUS_SUCCESS) {
        UserInfo user;
        user.user_id = parser.getString("user_id");
        user.username = parser.getString("username");
        user.email = parser.getString("email");

        g_session.setCurrentUser(user);

        cout << "SUCCESS: Logged in as " << user.username << "!" << endl;
    } else if (status == STATUS_INVALID_OTP) {
        cout << "ERROR: Invalid OTP code!" << endl;
    } else if (status == STATUS_OTP_EXPIRED) {
        cout << "ERROR: OTP expired!" << endl;
    } else {
        cout << "ERROR: Login failed!" << endl;
    }
}

// Get projects
void handleGetProjects() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string response;
    cout << "Getting projects..." << endl;

    if (!g_network.getProjects(response)) {
        cout << "ERROR: Failed to get projects!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status != STATUS_SUCCESS) {
        cout << "ERROR: Failed to get projects!" << endl;
        return;
    }

    cout << "\n=== YOUR PROJECTS ===" << endl;
    cout << "Raw response: " << response << endl;
    cout << "\nNote: Parse the 'projects' array to display nicely" << endl;
}

// Create project
void handleCreateProject() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string name, description;

    cout << "\n=== CREATE PROJECT ===" << endl;
    cin.ignore();
    cout << "Project name: ";
    getline(cin, name);
    cout << "Description: ";
    getline(cin, description);

    string response;
    cout << "Creating project..." << endl;

    if (!g_network.createProject(name, description, response)) {
        cout << "ERROR: Failed to create project!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status == STATUS_SUCCESS) {
        string project_id = parser.getString("project_id");
        cout << "SUCCESS: Project created with ID: " << project_id << endl;
        g_session.setCurrentProject(project_id);
        cout << "Current project set to: " << project_id << endl;
    } else {
        cout << "ERROR: Failed to create project!" << endl;
    }
}

// Get tasks
void handleGetTasks() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string project_id = g_session.getCurrentProjectId();

    if (project_id.empty()) {
        cout << "Enter project ID: ";
        cin >> project_id;
        g_session.setCurrentProject(project_id);
    }

    string response;
    cout << "Getting tasks for project: " << project_id << endl;

    if (!g_network.getTasks(project_id, response)) {
        cout << "ERROR: Failed to get tasks!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status != STATUS_SUCCESS) {
        cout << "ERROR: Failed to get tasks!" << endl;
        return;
    }

    cout << "\n=== TASKS ===" << endl;
    cout << "Raw response: " << response << endl;
    cout << "\nNote: Parse the 'tasks' array to display nicely" << endl;
}

// Create task
void handleCreateTask() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string project_id = g_session.getCurrentProjectId();

    if (project_id.empty()) {
        cout << "Enter project ID: ";
        cin >> project_id;
        g_session.setCurrentProject(project_id);
    }

    string name, description, priority, due_date;

    cout << "\n=== CREATE TASK ===" << endl;
    cin.ignore();
    cout << "Task name: ";
    getline(cin, name);
    cout << "Description: ";
    getline(cin, description);
    cout << "Priority (low/medium/high/urgent): ";
    cin >> priority;
    cout << "Due date (YYYY-MM-DD): ";
    cin >> due_date;

    string response;
    cout << "Creating task..." << endl;

    if (!g_network.createTask(project_id, name, description, "", priority, due_date, response)) {
        cout << "ERROR: Failed to create task!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status == STATUS_SUCCESS) {
        string task_id = parser.getString("task_id");
        cout << "SUCCESS: Task created with ID: " << task_id << endl;
    } else {
        cout << "ERROR: Failed to create task!" << endl;
    }
}

// Update task status
void handleUpdateTaskStatus() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string task_id, new_status;

    cout << "\n=== UPDATE TASK STATUS ===" << endl;
    cout << "Task ID: ";
    cin >> task_id;
    cout << "New status (todo/in_progress/completed/in_review/blocked): ";
    cin >> new_status;

    string response;
    cout << "Updating task..." << endl;

    if (!g_network.updateTask(task_id, "", "", "", new_status, "", "", response)) {
        cout << "ERROR: Failed to update task!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status == STATUS_SUCCESS) {
        cout << "SUCCESS: Task status updated!" << endl;
    } else {
        cout << "ERROR: Failed to update task!" << endl;
    }
}

// Send chat message
void handleSendChat() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string project_id = g_session.getCurrentProjectId();

    if (project_id.empty()) {
        cout << "Enter project ID: ";
        cin >> project_id;
        g_session.setCurrentProject(project_id);
    }

    string message;

    cout << "\n=== SEND CHAT MESSAGE ===" << endl;
    cin.ignore();
    cout << "Message: ";
    getline(cin, message);

    string response;
    cout << "Sending message..." << endl;

    if (!g_network.sendChatMessage(project_id, message, response)) {
        cout << "ERROR: Failed to send message!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status == STATUS_SUCCESS) {
        cout << "SUCCESS: Message sent!" << endl;
    } else {
        cout << "ERROR: Failed to send message!" << endl;
    }
}

// View chat history
void handleViewChat() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string project_id = g_session.getCurrentProjectId();

    if (project_id.empty()) {
        cout << "Enter project ID: ";
        cin >> project_id;
        g_session.setCurrentProject(project_id);
    }

    string response;
    cout << "Getting chat history..." << endl;

    if (!g_network.getChatHistory(project_id, 50, response)) {
        cout << "ERROR: Failed to get chat history!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status != STATUS_SUCCESS) {
        cout << "ERROR: Failed to get chat history!" << endl;
        return;
    }

    cout << "\n=== CHAT HISTORY ===" << endl;
    cout << "Raw response: " << response << endl;
    cout << "\nNote: Parse the 'messages' array to display nicely" << endl;
}

// Add contact
void handleAddContact() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string contact;

    cout << "\n=== ADD CONTACT ===" << endl;
    cout << "Username or email: ";
    cin >> contact;

    string response;
    cout << "Adding contact..." << endl;

    if (!g_network.addContact(contact, response)) {
        cout << "ERROR: Failed to add contact!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status == STATUS_SUCCESS) {
        cout << "SUCCESS: Contact added!" << endl;
    } else if (status == STATUS_USER_NOT_FOUND) {
        cout << "ERROR: User not found!" << endl;
    } else {
        cout << "ERROR: Failed to add contact!" << endl;
    }
}

// View contacts
void handleViewContacts() {
    if (!g_session.isLoggedIn()) {
        cout << "ERROR: Please login first!" << endl;
        return;
    }

    string response;
    cout << "Getting contacts..." << endl;

    if (!g_network.getContacts(response)) {
        cout << "ERROR: Failed to get contacts!" << endl;
        return;
    }

    JsonParser parser(response);
    int status = parser.getInt("status");

    if (status != STATUS_SUCCESS) {
        cout << "ERROR: Failed to get contacts!" << endl;
        return;
    }

    cout << "\n=== YOUR CONTACTS ===" << endl;
    cout << "Raw response: " << response << endl;
    cout << "\nNote: Parse the 'contacts' array to display nicely" << endl;
}

// Handle logout
void handleLogout() {
    string response;
    g_network.logout(response);
    g_session.clearSession();
    cout << "Logged out successfully!" << endl;
}

int main() {
    cout << "=======================================" << endl;
    cout << "  Task Management Client" << endl;
    cout << "  Server: 127.0.0.1:8080" << endl;
    cout << "=======================================" << endl;

    // Connect to server
    cout << "Connecting to server..." << endl;
    if (!g_network.connect()) {
        cout << "ERROR: Cannot connect to server!" << endl;
        cout << "Make sure the server is running on 127.0.0.1:8080" << endl;
        return 1;
    }

    cout << "Connected to server successfully!" << endl;

    int choice;

    while (true) {
        displayMenu();
        cin >> choice;

        switch (choice) {
            case 0:
                cout << "Goodbye!" << endl;
                g_network.disconnect();
                return 0;

            case 1:
                handleRegister();
                break;

            case 2:
                handleLogin();
                break;

            case 3:
                handleGetProjects();
                break;

            case 4:
                handleCreateProject();
                break;

            case 5:
                handleGetTasks();
                break;

            case 6:
                handleCreateTask();
                break;

            case 7:
                handleUpdateTaskStatus();
                break;

            case 8:
                handleSendChat();
                break;

            case 9:
                handleViewChat();
                break;

            case 10:
                handleAddContact();
                break;

            case 11:
                handleViewContacts();
                break;

            case 12:
                handleLogout();
                break;

            default:
                cout << "Invalid choice!" << endl;
        }

        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
    }

    return 0;
}
