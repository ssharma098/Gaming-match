import { createUser, login } from "./database.js";

// SIGN UP user
const newUser = createUser("john", 22, ["music", "games"], "London");
console.log("New Account Created:", newUser);

// LOGIN user
const userLogin = login("john", newUser.password);
console.log("Login Result:", userLogin);
