# 调用关系图（Call Graph）

此目录包含项目中核心类 / 场景 / 管理器之间的调用关系可视化文件。

- `callgraph.mmd` — Mermaid 源文件（可以在 mermaid.live 或 mermaid-cli 中渲染）
- `callgraph.svg` — 基本的静态 SVG 图像（在浏览器或 Markdown 渲染中可查看）

## 渲染方法

1. 使用 mermaid.live 在线渲染：将 `callgraph.mmd` 的内容复制到 https://mermaid.live/ 即可预览并导出 PNG/SVG。

2. 使用 `mmdc` (Mermaid CLI) 本地渲染：

```powershell
# 安装 mermaid-cli (需要 node/npm)
npm install -g @mermaid-js/mermaid-cli

# 渲染为 PNG
mmdc -i docs\diagrams\callgraph.mmd -o docs\diagrams\callgraph.png

# 渲染为 SVG
mmdc -i docs\diagrams\callgraph.mmd -o docs\diagrams\callgraph.svg
```

3. 如果你想通过命令行自动化完成（在 Windows PowerShell 下示例）：

```powershell
if (!(Get-Command mmdc -ErrorAction SilentlyContinue)) {
  Write-Host "Please install mermaid-cli: npm install -g @mermaid-js/mermaid-cli"
}
else {
  mmdc -i docs\diagrams\callgraph.mmd -o docs\diagrams\callgraph.png
}
```

## 说明

- 该 SVG 文件是手工生成的静态示例，基于 `callgraph.mmd` 的内容绘制。如果你希望得到更精确或更复杂的布局（例如每条边注文本行号、函数名），建议使用 Mermaid CLI 或 mermaid.live 渲染并在渲染参数中启用更复杂的样式。
- 若你希望我把渲染后的 PNG 或改进后的 SVG 导入项目仓库并提交，请告诉我，我可以继续操作。
