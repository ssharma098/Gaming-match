import fs from "fs";

// load database
function loadDB() {
    const data = fs.readFileSync("database.json", "utf8");
    return JSON.parse(data);
}

// save database
function saveDB(db) {
    fs.writeFileSync("database.json", JSON.stringify(db, null, 2));
}

// generate 6-char password
function generatePassword() {
    const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    let pass = "";
    for (let i = 0; i < 6; i++) {
        pass += chars[Math.floor(Math.random() * chars.length)];
    }
    return pass;
}

// create user
export function createUser(username, age, hobbies, city) {
    const db = loadDB();

    const userId = "U" + Math.floor(100000 + Math.random() * 900000);
    const password = generatePassword();

    const user = {
        userId,
        username,
        password,
        age,
        hobbies,
        city,
        level: 1,
        coins: 0
    };

    db.users.push(user);
    saveDB(db);

    return user;
}

// login
export function login(username, password) {
    const db = loadDB();

    const user = db.users.find(u => u.username === username);
    if (!user) return null;

    if (user.password !== password) return false;

    return user;
}
