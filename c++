// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>

static int exec_sql(sqlite3* db, const std::string &sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << (err?err:"unknown") << "\n";
        sqlite3_free(err);
    }
    return rc;
}

void create_schema(sqlite3* db) {
    const char* schema_sql = R"sql(
    PRAGMA foreign_keys = ON;
    CREATE TABLE IF NOT EXISTS users (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      username TEXT NOT NULL UNIQUE,
      display_name TEXT,
      email TEXT UNIQUE,
      password_hash TEXT,
      birthdate TEXT,
      gender TEXT,
      bio TEXT,
      city TEXT,
      country TEXT,
      created_at TEXT DEFAULT (datetime('now'))
    );
    CREATE TABLE IF NOT EXISTS games (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      name TEXT NOT NULL UNIQUE
    );
    CREATE TABLE IF NOT EXISTS user_games (
      user_id INTEGER NOT NULL,
      game_id INTEGER NOT NULL,
      skill_level TEXT,
      PRIMARY KEY (user_id, game_id),
      FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
      FOREIGN KEY (game_id) REFERENCES games(id) ON DELETE CASCADE
    );
    CREATE TABLE IF NOT EXISTS preferences (
      user_id INTEGER PRIMARY KEY,
      preferred_gender TEXT,
      min_age INTEGER,
      max_age INTEGER,
      max_distance_km INTEGER,
      FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    );
    )sql";

    exec_sql(db, schema_sql);
}

int add_user(sqlite3* db, const std::string &username, const std::string &display_name,
             const std::string &email, const std::string &birthdate, const std::string &gender,
             const std::string &city, const std::string &country, const std::string &bio) 
{
    const char* sql = "INSERT INTO users (username,display_name,email,birthdate,gender,city,country,bio) VALUES (?,?,?,?,?,?,?,?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "prepare failed\n"; return -1;
    }
    sqlite3_bind_text(stmt,1,username.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,display_name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,email.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4,birthdate.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,5,gender.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,6,city.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,7,country.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,8,bio.c_str(),-1,SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "insert failed: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(db);
}

int add_game_if_missing(sqlite3* db, const std::string &game_name) {
    // insert or ignore
    const char* ins = "INSERT OR IGNORE INTO games (name) VALUES (?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, ins, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt,1,game_name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // select id
    const char* sel = "SELECT id FROM games WHERE name = ?;";
    sqlite3_prepare_v2(db, sel, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt,1,game_name.c_str(),-1,SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    int id = -1;
    if (rc == SQLITE_ROW) id = sqlite3_column_int(stmt,0);
    sqlite3_finalize(stmt);
    return id;
}

void add_user_game(sqlite3* db, int user_id, int game_id, const std::string &skill) {
    const char* sql = "INSERT OR IGNORE INTO user_games (user_id,game_id,skill_level) VALUES (?,?,?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt,1,user_id);
    sqlite3_bind_int(stmt,2,game_id);
    sqlite3_bind_text(stmt,3,skill.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// Simple match query: shared games count
void find_matches(sqlite3* db, int user_id, int limit = 10) {
    const char* sql = R"sql(
    SELECT u.id, u.username, u.display_name, COUNT(*) AS shared_games
    FROM users u
    JOIN user_games ug ON u.id = ug.user_id
    WHERE ug.game_id IN (SELECT game_id FROM user_games WHERE user_id = ?)
      AND u.id <> ?
    GROUP BY u.id
    ORDER BY shared_games DESC, u.created_at DESC
    LIMIT ?;
    )sql";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt,1,user_id);
    sqlite3_bind_int(stmt,2,user_id);
    sqlite3_bind_int(stmt,3,limit);

    std::cout << "Matches for user_id="<<user_id<<":\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt,0);
        const unsigned char* uname = sqlite3_column_text(stmt,1);
        const unsigned char* dname = sqlite3_column_text(stmt,2);
        int shared = sqlite3_column_int(stmt,3);
        std::cout << "  id="<<id<<" username="<<(uname? (const char*)uname : "") 
                  <<" display="<<(dname? (const char*)dname : "") 
                  <<" shared_games="<<shared<<"\n";
    }
    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db;
    if (sqlite3_open("gamers.db",&db) != SQLITE_OK) {
        std::cerr << "Cannot open db\n"; return 1;
    }
    create_schema(db);

    int alice = add_user(db,"alice123","Alice","alice@example.com","1996-07-12","female","Plymouth","UK","Casual FPS fan.");
    int bob = add_user(db,"bobGamer","Bob","bob@example.com","1993-02-03","male","Plymouth","UK","Competitive jungler.");

    int g_val = add_game_if_missing(db,"Valorant");
    int g_mc  = add_game_if_missing(db,"Minecraft");
    int g_lol = add_game_if_missing(db,"League of Legends");

    add_user_game(db, alice, g_val, "intermediate");
    add_user_game(db, alice, g_mc, "casual");
    add_user_game(db, bob, g_val, "high");
    add_user_game(db, bob, g_lol, "high");

    find_matches(db, alice);

    sqlite3_close(db);
    return 0;
}