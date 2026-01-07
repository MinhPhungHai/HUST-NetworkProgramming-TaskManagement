#!/bin/bash

# PostgreSQL Database Setup Script for Network Programming Project
# This script will create the database user, database, and schema

echo "======================================"
echo "Network Programming Database Setup"
echo "======================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Database configuration
DB_NAME="netprog_db"
DB_USER="netprog_user"
DB_PASSWORD="netprog_password"  # Change this to a secure password
SCHEMA_NAME="netprog"

echo -e "${YELLOW}This script will:${NC}"
echo "1. Create a PostgreSQL user: $DB_USER"
echo "2. Create a database: $DB_NAME"
echo "3. Create schema: $SCHEMA_NAME with all tables"
echo ""
echo -e "${YELLOW}Note: You may need to enter your sudo password${NC}"
echo ""

# Create PostgreSQL user and database
echo -e "${GREEN}Step 1: Creating PostgreSQL user...${NC}"
sudo -u postgres psql <<EOF
-- Create user if not exists
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_user WHERE usename = '$DB_USER') THEN
        CREATE USER $DB_USER WITH PASSWORD '$DB_PASSWORD';
    END IF;
END
\$\$;

-- Create database if not exists
SELECT 'CREATE DATABASE $DB_NAME OWNER $DB_USER'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = '$DB_NAME')\gexec

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;
ALTER USER $DB_USER CREATEDB;

\q
EOF

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ User and database created successfully${NC}"
else
    echo -e "${RED}✗ Failed to create user and database${NC}"
    exit 1
fi

# Create schema and tables
echo ""
echo -e "${GREEN}Step 2: Creating schema and tables...${NC}"
sudo -u postgres psql -d $DB_NAME -f "$(dirname "$0")/schema.sql"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Schema created successfully${NC}"
else
    echo -e "${RED}✗ Failed to create schema${NC}"
    exit 1
fi

# Grant permissions on schema
echo ""
echo -e "${GREEN}Step 3: Setting up permissions...${NC}"
sudo -u postgres psql -d $DB_NAME <<EOF
GRANT ALL PRIVILEGES ON SCHEMA $SCHEMA_NAME TO $DB_USER;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA $SCHEMA_NAME TO $DB_USER;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA $SCHEMA_NAME TO $DB_USER;
ALTER DEFAULT PRIVILEGES IN SCHEMA $SCHEMA_NAME GRANT ALL ON TABLES TO $DB_USER;
ALTER DEFAULT PRIVILEGES IN SCHEMA $SCHEMA_NAME GRANT ALL ON SEQUENCES TO $DB_USER;
\q
EOF

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Permissions set successfully${NC}"
else
    echo -e "${RED}✗ Failed to set permissions${NC}"
    exit 1
fi

# Create connection info file
echo ""
echo -e "${GREEN}Step 4: Creating connection info file...${NC}"
cat > "$(dirname "$0")/db_connection_info.txt" <<EOF
Database Connection Information
================================

Database Name: $DB_NAME
Username: $DB_USER
Password: $DB_PASSWORD
Host: localhost
Port: 5432
Schema: $SCHEMA_NAME

Connection String (for application):
postgresql://$DB_USER:$DB_PASSWORD@localhost:5432/$DB_NAME

Connection command:
psql -U $DB_USER -d $DB_NAME

After connecting, set the schema:
SET search_path TO $SCHEMA_NAME;

Or connect directly to schema:
psql -U $DB_USER -d $DB_NAME -c "SET search_path TO $SCHEMA_NAME;"

SECURITY NOTE:
- Change the default password in production!
- Keep this file secure and don't commit it to version control
- Add db_connection_info.txt to .gitignore
EOF

echo -e "${GREEN}✓ Connection info saved to db_connection_info.txt${NC}"

# Test connection
echo ""
echo -e "${GREEN}Step 5: Testing connection...${NC}"
PGPASSWORD=$DB_PASSWORD psql -U $DB_USER -d $DB_NAME -c "SET search_path TO $SCHEMA_NAME; SELECT table_name FROM information_schema.tables WHERE table_schema = '$SCHEMA_NAME' ORDER BY table_name;"

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}======================================"
    echo "✓ Database setup completed successfully!"
    echo "======================================${NC}"
    echo ""
    echo "You can now connect to the database using:"
    echo "  psql -U $DB_USER -d $DB_NAME"
    echo ""
    echo "Connection details saved in: db_connection_info.txt"
else
    echo -e "${RED}✗ Connection test failed${NC}"
    exit 1
fi
