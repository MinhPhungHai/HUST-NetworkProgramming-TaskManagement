#!/bin/bash

# PostgreSQL Database Setup Script - Network Programming Project
# This script handles complete database setup: user, database, schema, and authentication

echo "=========================================="
echo "  PostgreSQL Database Setup"
echo "  Network Programming Task Management"
echo "=========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Database configuration
DB_NAME="netprog_db"
DB_USER="netprog_user"
DB_PASSWORD="netprog_password"
SCHEMA_NAME="netprog"

echo -e "${YELLOW}This script will:${NC}"
echo "  1. Create PostgreSQL user: $DB_USER"
echo "  2. Create database: $DB_NAME"
echo "  3. Create schema: $SCHEMA_NAME with all tables"
echo "  4. Configure authentication"
echo "  5. Setup passwordless access (.pgpass)"
echo ""
echo -e "${YELLOW}Note: You will be prompted for your sudo password${NC}"
echo ""
read -p "Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Setup cancelled."
    exit 0
fi

# ============================================
# STEP 1: Create PostgreSQL user and database
# ============================================
echo ""
echo -e "${GREEN}[1/5] Creating PostgreSQL user and database...${NC}"

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

# ============================================
# STEP 2: Configure PostgreSQL Authentication
# ============================================
echo ""
echo -e "${GREEN}[2/5] Configuring PostgreSQL authentication...${NC}"

# Find PostgreSQL version and config file
PG_VERSION=$(ls /etc/postgresql/ | head -1)
PG_HBA_CONF="/etc/postgresql/$PG_VERSION/main/pg_hba.conf"

echo -e "${YELLOW}PostgreSQL Version: $PG_VERSION${NC}"

# Backup the config
sudo cp "$PG_HBA_CONF" "$PG_HBA_CONF.backup.$(date +%Y%m%d_%H%M%S)" 2>/dev/null

# Create new config with our rules at the top
cat > /tmp/pg_hba_new.conf << 'EOF'
# Network Programming Database - Password Authentication
local   netprog_db      netprog_user                            md5
host    netprog_db      netprog_user    127.0.0.1/32            md5
host    netprog_db      netprog_user    ::1/128                 md5

EOF

# Append original config (without our previous additions)
sudo grep -v "Network Programming Database" "$PG_HBA_CONF" >> /tmp/pg_hba_new.conf

# Replace config
sudo cp /tmp/pg_hba_new.conf "$PG_HBA_CONF"
rm -f /tmp/pg_hba_new.conf

# Restart PostgreSQL
sudo systemctl restart postgresql
sleep 2

echo -e "${GREEN}✓ Authentication configured${NC}"

# ============================================
# STEP 3: Create schema and tables
# ============================================
echo ""
echo -e "${GREEN}[3/5] Creating database schema and tables...${NC}"

# Get absolute path to schema.sql
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SCHEMA_FILE="$SCRIPT_DIR/schema.sql"

if [ ! -f "$SCHEMA_FILE" ]; then
    echo -e "${RED}✗ Schema file not found: $SCHEMA_FILE${NC}"
    exit 1
fi

# Copy to /tmp for postgres user access
cp "$SCHEMA_FILE" /tmp/netprog_schema.sql
chmod 644 /tmp/netprog_schema.sql

# Create schema
sudo -u postgres psql -d $DB_NAME -f /tmp/netprog_schema.sql > /tmp/schema_output.log 2>&1

# Clean up
rm -f /tmp/netprog_schema.sql

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Schema created successfully${NC}"
    echo -e "${YELLOW}  Created 10 tables with indexes and triggers${NC}"
else
    echo -e "${RED}✗ Failed to create schema${NC}"
    cat /tmp/schema_output.log
    rm -f /tmp/schema_output.log
    exit 1
fi
rm -f /tmp/schema_output.log

# Grant permissions
sudo -u postgres psql -d $DB_NAME <<EOF > /dev/null 2>&1
GRANT ALL PRIVILEGES ON SCHEMA $SCHEMA_NAME TO $DB_USER;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA $SCHEMA_NAME TO $DB_USER;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA $SCHEMA_NAME TO $DB_USER;
ALTER DEFAULT PRIVILEGES IN SCHEMA $SCHEMA_NAME GRANT ALL ON TABLES TO $DB_USER;
ALTER DEFAULT PRIVILEGES IN SCHEMA $SCHEMA_NAME GRANT ALL ON SEQUENCES TO $DB_USER;
\q
EOF

# ============================================
# STEP 4: Setup passwordless access (.pgpass)
# ============================================
echo ""
echo -e "${GREEN}[4/5] Setting up passwordless access...${NC}"

PGPASS_FILE="$HOME/.pgpass"

# Backup existing .pgpass if present
if [ -f "$PGPASS_FILE" ]; then
    cp "$PGPASS_FILE" "$PGPASS_FILE.backup.$(date +%Y%m%d_%H%M%S)" 2>/dev/null
fi

# Remove old netprog entries
if [ -f "$PGPASS_FILE" ]; then
    sed -i '/netprog_db:netprog_user/d' "$PGPASS_FILE" 2>/dev/null
fi

# Add new entry
echo "localhost:5432:netprog_db:netprog_user:netprog_password" >> "$PGPASS_FILE"
chmod 600 "$PGPASS_FILE"

echo -e "${GREEN}✓ Passwordless access configured${NC}"

# ============================================
# STEP 5: Create connection info file
# ============================================
echo ""
echo -e "${GREEN}[5/5] Creating connection info file...${NC}"

cat > "$SCRIPT_DIR/db_connection_info.txt" <<EOF
Database Connection Information
================================

Database Name: $DB_NAME
Username: $DB_USER
Password: $DB_PASSWORD
Host: localhost
Port: 5432
Schema: $SCHEMA_NAME

Connection String:
postgresql://$DB_USER:$DB_PASSWORD@localhost:5432/$DB_NAME

psql Command:
psql -U $DB_USER -d $DB_NAME -h localhost

Python (psycopg2):
conn = psycopg2.connect(
    dbname="$DB_NAME",
    user="$DB_USER",
    password="$DB_PASSWORD",
    host="localhost",
    port="5432"
)

Java (JDBC):
String url = "jdbc:postgresql://localhost:5432/$DB_NAME?currentSchema=$SCHEMA_NAME";
Connection conn = DriverManager.getConnection(url, "$DB_USER", "$DB_PASSWORD");

Node.js (pg):
const pool = new Pool({
    host: 'localhost',
    database: '$DB_NAME',
    user: '$DB_USER',
    password: '$DB_PASSWORD'
});

SECURITY NOTE:
- This file is auto-generated and excluded from git
- Change the password in production!
- Keep this file secure
EOF

echo -e "${GREEN}✓ Connection info saved${NC}"

# ============================================
# STEP 6: Test connection
# ============================================
echo ""
echo -e "${GREEN}Testing database connection...${NC}"

psql -U $DB_USER -d $DB_NAME -h localhost -c "SET search_path TO $SCHEMA_NAME; SELECT COUNT(*) as table_count FROM information_schema.tables WHERE table_schema = '$SCHEMA_NAME';" 2>&1 | grep -q "10"

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}=========================================="
    echo "  ✓ Database Setup Complete!"
    echo "==========================================${NC}"
    echo ""
    echo "Database Details:"
    echo "  Name: $DB_NAME"
    echo "  User: $DB_USER"
    echo "  Schema: $SCHEMA_NAME"
    echo "  Tables: 10 (users, projects, tasks, chat, etc.)"
    echo ""
    echo "Connect to database:"
    echo "  psql -U $DB_USER -d $DB_NAME -h localhost"
    echo ""
    echo "Next steps:"
    echo "  • Load sample data: ./load_sample_data.sh"
    echo "  • View connection details: cat db_connection_info.txt"
    echo "  • Check README.md for usage examples"
    echo ""
else
    echo -e "${RED}✗ Connection test failed${NC}"
    echo ""
    echo "Database was created but connection test failed."
    echo "Try connecting manually:"
    echo "  psql -U $DB_USER -d $DB_NAME -h localhost"
    exit 1
fi
