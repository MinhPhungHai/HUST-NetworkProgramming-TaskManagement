#!/bin/bash

# Load Sample Data Script - Network Programming Project
# This script loads sample data into the database for testing

echo "=========================================="
echo "  Load Sample Data"
echo "  Network Programming Task Management"
echo "=========================================="
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

DB_NAME="netprog_db"
DB_USER="netprog_user"

# Check if database exists
echo -e "${YELLOW}Checking database connection...${NC}"
psql -U $DB_USER -d $DB_NAME -h localhost -c "\q" 2>/dev/null

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Cannot connect to database '$DB_NAME'${NC}"
    echo -e "${YELLOW}Please run ./setup_database.sh first${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Database connection successful${NC}"
echo ""

# Get absolute path to sample_data.sql
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SAMPLE_FILE="$SCRIPT_DIR/sample_data.sql"

if [ ! -f "$SAMPLE_FILE" ]; then
    echo -e "${RED}✗ Sample data file not found: $SAMPLE_FILE${NC}"
    exit 1
fi

# Load sample data
echo -e "${GREEN}Loading sample data...${NC}"

# Copy to /tmp for postgres user access
cp "$SAMPLE_FILE" /tmp/netprog_sample_data.sql
chmod 644 /tmp/netprog_sample_data.sql

# Import data
psql -U $DB_USER -d $DB_NAME -h localhost -f /tmp/netprog_sample_data.sql > /tmp/sample_output.log 2>&1

# Clean up
rm -f /tmp/netprog_sample_data.sql

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}=========================================="
    echo "  ✓ Sample Data Loaded Successfully!"
    echo "==========================================${NC}"
    echo ""
    echo "Sample data includes:"
    echo "  • 8 users (john_doe, jane_smith, etc.)"
    echo "  • 6 projects (Network Programming, E-Commerce, etc.)"
    echo "  • 20 tasks with various statuses"
    echo "  • 15 task comments"
    echo "  • 27 chat messages"
    echo "  • 14 file attachments"
    echo "  • 19 notifications"
    echo "  • Friend connections and OTP records"
    echo ""
    echo "Test credentials:"
    echo "  Username: john_doe"
    echo "  Password: password123"
    echo ""
    echo "Connect and explore:"
    echo "  psql -U $DB_USER -d $DB_NAME -h localhost"
    echo ""
    echo "Try these queries:"
    echo "  SET search_path TO netprog;"
    echo "  SELECT * FROM projects;"
    echo "  SELECT * FROM users;"
    echo ""
else
    echo -e "${RED}✗ Failed to load sample data${NC}"
    echo ""
    echo "Error output:"
    cat /tmp/sample_output.log
    rm -f /tmp/sample_output.log
    exit 1
fi

rm -f /tmp/sample_output.log
