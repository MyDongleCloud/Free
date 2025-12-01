import { Component, ViewChild, ChangeDetectorRef } from '@angular/core';
import { IonModal } from '@ionic/angular';
import { HttpClient } from '@angular/common/http';
import { Global } from '../env';
import { ModalController } from '@ionic/angular';

interface FileSystemNode {
	name: string;
	type: "directory" | "file" | "symlink";
	permissions: string;
	owner: string;
	group: string;
	size: number;
	modified: string;
	children?: FileSystemNode[];
}

@Component({
	selector: 'app-serverlog',
	templateUrl: './serverlog.page.html',
	standalone: false
})

export class ServerLog {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }
nodes: FileSystemNode[] = [];
expandedNodes = new Set<string>();
currentPath: string = "/";
@ViewChild("modalContent") modalContent: IonModal;
modalContentName: string;
modalContentMeta: string;
modalContentText: string;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient, private modalController: ModalController ) {}

async ionViewDidEnter() {
	await this.loadDirectory(this.currentPath);
}

async loadDirectory(path: string) {
	try {
		const data = { path };
		this.nodes = [await this.httpClient.post<FileSystemNode>("/_app_/auth/server-log", JSON.stringify(data), { headers: { "content-type": "application/json" } }).toPromise()];
		this.toggleExpand(this.nodes[0]);
		this.cdr.detectChanges();
	} catch (error) {
		console.error("Failed to load directory:", error);
	}
}

async getFileContent(fileNode: FileSystemNode): Promise<string> {
	const fullPath = this.buildFullPath(fileNode);
	const data = { path: fullPath };
	return await this.httpClient.post( "/_app_/auth/server-log", JSON.stringify(data), { headers: { "content-type": "application/json" }, responseType: "text" }).toPromise();
}

getSizeString(n: number): string {
	if (n === undefined || n === null) return "0B";
	const units = ["B", "KB", "MB", "GB", "TB", "PB"];
	let size = parseFloat(n.toString());
	let unitIndex = 0;
	while (size >= 1024 && unitIndex < units.length - 1) {
		size /= 1024;
		unitIndex++;
	}
	return size.toFixed(unitIndex === 0 ? 0 : 1) + " " + units[unitIndex];
}

async viewFileContent(fileNode: FileSystemNode) {
	if (fileNode.type !== "file") return;
	try {
		this.modalContentText = await this.getFileContent(fileNode);
		this.modalContentName = fileNode.name;
		this.modalContentMeta = "("+ this.getSizeString(fileNode.size) + ")";
		await this.modalContent.present();
	} catch (error) {
		console.error("Failed to load file content:", error);
		this.global.presentAlert("Error", "Failed to load file content", "Error");
	}
}

closeContent() {
	this.modalContent.dismiss();
}

private buildFullPath(node: FileSystemNode): string {
	if (this.nodes[0] === node)
		return "";//`/${node.name}`;
	const parentNode = this.findParentNode(node, this.nodes);
	if (parentNode) {
		const parentPath = this.buildFullPath(parentNode);
		return `${parentPath}/${node.name}`;
	}
	return `/${node.name}`;
}

private findParentNode(nodeToFind: FileSystemNode, tree: FileSystemNode[]): FileSystemNode | null {
	for (const node of tree) {
		if (node.children && node.children.includes(nodeToFind))
			return node;
		if (node.children) {
			const found = this.findParentNode(nodeToFind, node.children);
			if (found)
				return found;
		}
	}
	return null;
}

toggleExpand(node: FileSystemNode) {
	const path = this.getPath(node);
	if (this.expandedNodes.has(path))
		this.expandedNodes.delete(path);
	else
		this.expandedNodes.add(path);
}

isExpanded(node: FileSystemNode): boolean {
	return this.expandedNodes.has(this.getPath(node));
}

private getPath(node: FileSystemNode): string {
	return this.buildFullPath(node);
}

}
