#!/bin/bash

# Load Sample Data Script
# This script will load sample data into the netprog database

echo "======================================"
echo "Loading Sample Data"
echo "======================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

DB_NAME="netprog_db"
DB_USER="netprog_user"

# Check if database exists
echo -e "${YELLOW}Checking database connection...${NC}"
sudo -u postgres psql -d $DB_NAME -c "\q" 2>/dev/null

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Database '$DB_NAME' not found!${NC}"
    echo -e "${YELLOW}Please run setup_database.sh first to create the database.${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Database connection successful${NC}"
echo ""

# Load sample data
echo -e "${GREEN}Loading sample data...${NC}"
sudo -u postgres psql -d $DB_NAME -f "$(dirname "$0")/sample_data.sql"

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}======================================"
    echo "✓ Sample data loaded successfully!"
    echo "======================================${NC}"
    echo ""
    echo "Sample data includes:"
    echo "  • 8 users (john_doe, jane_smith, mike_wilson, etc.)"
    echo "  • 6 projects (Network Programming, E-Commerce, etc.)"
    echo "  • 20 tasks with various statuses"
    echo "  • 15 task comments"
    echo "  • 27 chat messages"
    echo "  • 14 file attachments"
    echo "  • 19 notifications"
    echo "  • Friend connections between users"
    echo "  • OTP verification records"
    echo ""
    echo "Try connecting and querying:"
    echo "  psql -U $DB_USER -d $DB_NAME"
    echo "  SET search_path TO netprog;"
    echo "  SELECT * FROM projects;"
    echo ""
else
    echo -e "${RED}✗ Failed to load sample data${NC}"
    exit 1
fi
