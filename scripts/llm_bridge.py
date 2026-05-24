import sys
import json
import urllib.request
import urllib.error

DEEPSEEK_API_KEY = "sk-xxxxxxxxxxxxxxxxxxxxxxxx" 

def call_ollama(prompt):
    url = "http://localhost:11434/api/generate"
    data = {
        "model": "qwen2.5:7b",
        "prompt": prompt,
        "stream": False
    }
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'), headers={'Content-Type': 'application/json'})
    try:
        response = urllib.request.urlopen(req, timeout=10) 
        result = json.loads(response.read().decode('utf-8'))
        return result.get("response", "").strip()
    except Exception:
        return None

def call_deepseek(prompt):
    url = "https://api.deepseek.com/chat/completions"
    data = {
        "model": "deepseek-chat",
        "messages": [
            {"role": "system", "content": prompt},
            {"role": "user", "content": ""}
        ],
        "temperature": 0.1
    }
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'))
    req.add_header('Content-Type', 'application/json')
    req.add_header('Authorization', f'Bearer {DEEPSEEK_API_KEY}')
    try:
        response = urllib.request.urlopen(req, timeout=10)
        result = json.loads(response.read().decode('utf-8'))
        return result['choices'][0]['message']['content'].strip()
    except Exception as e:
        return f"Error: 无法连接API ({str(e)})"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: 没有提供用户意图")
        sys.exit(1)
        
    user_intent = sys.argv[1]
    system_prompt = f"""你是一个运行在 Ubuntu 操作系统底层的高级命令翻译引擎。唯一任务是将用户的自然语言意图，精准翻译为极其专业的单行 Shell 命令，或者系统内置的扩展模块指令。

【严格规则】：
1. 输出必须只有一行命令，绝对不包含任何问候、解释或标点，不能使用 Markdown 代码块。
2. 善用管道符、grep、awk 等处理常规意图。
3. 【系统内置命令扩展】：如果用户意图是想运行操作系统课程的设计模块，请仅输出以下对应的指令：
   处理机调度实验 -> run_scheduler
   内存管理实验 -> run_memory
   进程同步模拟 -> run_concurrency
   文件系统模拟 -> run_fs

【示例】：
输入：帮我找出最大的3个文件
输出：ls -alS | head -n 4

输入：我想测试一下先来先服务调度算法
输出：run_scheduler

输入：{user_intent}
输出："""
    
    command = call_ollama(system_prompt)
    if not command:
        command = call_deepseek(system_prompt)
        
    command = command.replace("```bash", "").replace("```", "").replace("`", "").strip()
    print(command)
