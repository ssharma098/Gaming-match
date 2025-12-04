Dating Game App – Simple User Database (JavaScript)

This project contains a very basic setup for storing user accounts for a dating-style game.
It uses plain JavaScript and JSON files — no frameworks or databases — just simple code that anyone on the team can plug into the backend later.

📁 Project Files
database.js
This file handles creating user accounts and saving them to database.json.

What it currently supports:

Create a user
Auto-generate user IDs
Store username
Store password (6-digit / letters allowed)
Store age
Store city
Store hobbies
Save everything into database.json

database.json
This is where all user data is stored.
Every new account gets added into this file automatically.

Example of stored data:

{
  "users": [
    {
      "id": "USR12345",
      "username": "exampleUser",
      "password": "pass123",
      "age": 20,
      "city": "Plymouth",
      "hobbies": ["music", "games"]
    }
  ]
}

test.js
A small test file to try creating new users and check that the database works.

Run it with:

node test.js

🚀 How to Run This Project
Install Node.js (if not already installed)
Open Terminal inside your project folder
Run:
node test.js
Check database.json — new users will appear inside this file.

🔧 What the Team Can Build Next
Your teammates (frontend/backend) can use this file to:
Hook it up to sign-up forms
Connect it to the game logic
Replace the JSON file later with a real database (MongoDB, MySQL, etc.)

📌 Purpose of This Repo
This is just a starter database system.
It keeps the project organized and gives your team a working base to build on.
