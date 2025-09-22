import nodemailer from "nodemailer";

const EMAIL_APP_NAME = "MyDongle.Cloud";
const EMAIL_APP_URL = "https://mydongle.cloud";
const EMAIL_USER = "admin@mydongle.cloud";	

const transporter = nodemailer.createTransport({
	host: "localhost",
	port: 465,
	secure: true
});

const sendVerificationEmail = async (to, code) => {
	await transporter.sendMail({
	from: `"${EMAIL_APP_NAME} - Email Verification" <${EMAIL_USER}>`,
	to,
	subject: `Verify your email - ${EMAIL_APP_NAME}`,
	html: `
		<div style="font-family:sans-serif; line-height:1.5; max-width:600px; margin:0 auto;">
			<div style="background-color:#0054e9; padding:20px; text-align:center;">
				<h1 style="color:white; margin:0;">${EMAIL_APP_NAME}</h1>
			</div>
			<div style="padding:30px; background-color:#f9f9f9;">
				<h2 style="color:#333;">Email Verification Required</h2>
				<p>Hello,</p>
				<p>Please use the verification code below to verify your email address:</p>
				<div style="text-align:center; margin:30px 0;">
					<div style="background-color:#2dd55b; color:white; padding:15px 30px; display:inline-block; border-radius:8px; font-size:24px; font-weight:bold; letter-spacing:3px;">${code}</div>
				</div>
				<p style="color:#666;">This code will expire in 10 minutes for security reasons.</p>
				<p>If you didn't request this verification, please ignore this email.</p>
				<hr style="border:none; border-top:1px solid #eee; margin:30px 0;">
				<p>Thank you,<br/>The ${EMAIL_APP_NAME} Team</p>
				<p style="font-size:12px; color:#999;">Visit at <a href="${EMAIL_APP_URL}" target="_blank" style="color:#0054e9;">${EMAIL_APP_URL}</a></p>
			</div>
		</div>
	`,
	});
};

const sendPasswordResetVerificationEmail = async (to, code) => {
	await transporter.sendMail({
	from: `"${EMAIL_APP_NAME} - Password Reset" <${EMAIL_USER}>`,
	to,
	subject: `Reset Your Password - ${EMAIL_APP_NAME}`,
	html: `
		<div style="font-family:sans-serif; line-height:1.5; max-width:600px; margin:0 auto;">
			<div style="background-color:#0054e9; padding:20px; text-align:center;">
				<h1 style="color:white; margin:0;">${EMAIL_APP_NAME}</h1>
			</div>
			<div style="padding:30px; background-color:#f9f9f9;">
				<h2 style="color:#333;">Password Reset Request</h2>
				<p>Hello,</p>
				<p>We received a request to reset your password. Use the code below to proceed:</p>
				<div style="text-align:center; margin:30px 0;">
					<div style="background-color:#2dd55b; color:white; padding:15px 30px; display:inline-block; border-radius:8px; font-size:24px; font-weight:bold; letter-spacing:3px;">${code}</div>
				</div>
				<p style="color:#666;">This code is valid for 10 minutes. If you didn't request this, you can safely ignore this email.</p>
				<p><strong>Security Tip:</strong> Never share this code with anyone. Our team will never ask for this code.</p>
				<hr style="border:none; border-top:1px solid #eee; margin:30px 0;">
				<p>Thank you,<br/>The ${EMAIL_APP_NAME} Team</p>
				<p style="font-size:12px; color:#999;">
				Visit us at <a href="${EMAIL_APP_URL}" target="_blank" style="color:#ff6b6b;">${EMAIL_APP_URL}</a>
				</p>
			</div>
		</div>
	`,
	});
};

const sendSignInOTP = async (to, code) => {
	await transporter.sendMail({
	from: `"${EMAIL_APP_NAME} - Sign In" <${EMAIL_USER}>`,
	to,
	subject: `Your Sign In Code - ${EMAIL_APP_NAME}`,
	html: `
		<div style="font-family:sans-serif; line-height:1.5; max-width:600px; margin:0 auto;">
			<div style="background-color:#0054e9; padding:20px; text-align:center;">
				<h1 style="color:white; margin:0;">${EMAIL_APP_NAME}</h1>
			</div>
			<div style="padding:30px; background-color:#f9f9f9;">
				<h2 style="color:#333;">Sign In to Your Account</h2>
				<p>Hello,</p>
				<p>Use the code below to sign in to your ${EMAIL_APP_NAME} account:</p>
				<div style="text-align:center; margin:30px 0;">
					<div style="background-color:#2dd55b; color:white; padding:15px 30px; display:inline-block; border-radius:8px; font-size:24px; font-weight:bold; letter-spacing:3px;">${code}</div>
				</div>
				<p style="color:#666;">This code will expire in 10 minutes for security reasons.</p>
				<p>If you didn't try to sign in, please ignore this email and consider changing your password.</p>
				<hr style="border:none; border-top:1px solid #eee; margin:30px 0;">
				<p>Thank you,<br/>The ${EMAIL_APP_NAME} Team</p>
				<p style="font-size:12px; color:#999;">
				Visit us at <a href="${EMAIL_APP_URL}" target="_blank" style="color:#74b9ff;">${EMAIL_APP_URL}</a>
				</p>
			</div>
		</div>
	`,
	});
};

const sendVerificationEmailURL = async (to, url) => {
	await transporter.sendMail({
	from: `"${EMAIL_APP_NAME} - Email Verification" <${EMAIL_USER}>`,
	to,
	subject: `Verify your email - ${EMAIL_APP_NAME}`,
	html: `
		<div style="font-family:sans-serif; line-height:1.5; max-width:600px; margin:0 auto;">
			<div style="background-color:#0054e9; padding:20px; text-align:center;">
				<h1 style="color:white; margin:0;">${EMAIL_APP_NAME}</h1>
			</div>
			<div style="padding:30px; background-color:#f9f9f9;">
				<h2 style="color:#333;">Confirm Your Email Address</h2>
				<p>Hello,</p>
				<p>Click the button below to verify your email address and activate your account:</p>
				<div style="text-align:center; margin:30px 0;">
				<a href="${url}" target="_blank" style="background-color:#4caf50; color:white; padding:15px 30px; text-decoration:none; border-radius:8px; font-size:18px; font-weight:bold; display:inline-block;">Verify Email</a>
				</div>
				<p style="color:#666;">If the button doesn’t work, copy and paste the following URL into your browser:</p>
				<p style="word-break:break-all;"><a href="${url}" target="_blank" style="color:#4caf50;">${url}</a></p>
				<p>If you didn't sign up for this account, you can safely ignore this email.</p>
				<hr style="border:none; border-top:1px solid #eee; margin:30px 0;">
				<p>Thank you,<br/>The ${EMAIL_APP_NAME} Team</p>
				<p style="font-size:12px; color:#999;">
				Visit us at <a href="${EMAIL_APP_URL}" target="_blank" style="color:#4caf50;">${EMAIL_APP_URL}</a>
				</p>
			</div>
		</div>
	`,
	});
};

export { 
	sendVerificationEmail, 
	sendPasswordResetVerificationEmail, 
	sendSignInOTP,
	sendVerificationEmailURL 
};
