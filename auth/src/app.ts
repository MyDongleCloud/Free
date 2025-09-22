import "dotenv/config";
import { writeFileSync } from "fs";
import { toNodeHandler } from "better-auth/node";
import cookieParser from "cookie-parser";
import { auth, handler, jwkInit, port } from "./auth";
import express from "express";
import cors from "cors";

const app = express();
/*
console.log(auth);
app.use((req, res, next) => {
	console.log({ method:req.method, path:req.path, host:req.get("host"), headers:req.headers });
	next();
});
*/
app.use(cors({
	origin: ["*"],
	methods: ["GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"],
	allowedHeaders: ["Content-Type", "Authorization", "Cookie"],
	credentials: true
}));
app.use(express.json({ limit: "50mb" }));
app.use(express.urlencoded({ limit:"50mb", extended:true }));
app.use(cookieParser());
app.set("trust proxy", 1);
app.get("/MyDongleCloud/Auth", (req, res) => {
	res.json({ status: "healthy", timestamp: new Date().toISOString() });
});
app.use("/MyDongleCloud/Auth", toNodeHandler(handler));
app.listen(port, () => {
	jwkInit();
	console.log("Server running at http://localhost: " + port + " (" + (process.env.PRODUCTION === "true" ? "production" : "development") + ")");
});
