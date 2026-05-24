import sys
import json
import urllib.request
import urllib.error

# ================= 配置区 =================
# 云端深度求索 API Key 备用区
DEEPSEEK_API_KEY = "sk-xxxxxxxxxxxxxxxxxxxxxxxx" 
# ==========================================

def call_ollama(prompt):
    """调用本地部署的 Ollama 模型 (qwen2.5:7b)"""
    url = "http://localhost:11434/api/generate"
    # 这里已经更新为你本地正在运行的极致强大千问 2.5:7B 模型
    data = {
        "model": "qwen2.5:7b",
        "prompt": prompt,
        "stream": False
    }
    req = urllib.request.Request(url, data=json.dumps(data).encode('utf-8'), headers={'Content-Type': 'application/json'})
    try:
        response = urllib.request.urlopen(req, timeout=10) # 给本地推理留足响应时间
        result = json.loads(response.read().decode('utf-8'))
        return result.get("response", "").strip()
    except Exception:
        return None

def call_deepseek(prompt):
    """调用云端 DeepSeek API (作为本地模型的备用)"""
    url = "https://api.deepseek.com/chat/completions"
    data = {
        "model": "deepseek-chat",
        "messages": [
            {"role": "system", "content": "你是一个Linux系统命令翻译器。请将用户的自然语言意图翻译成单行有效的Linux命令。只能输出纯命令，不要任何Markdown格式，不要任何解释，不要加代码块反引号。"},
            {"role": "user", "content": prompt}
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
        return f"Error: 无法连接到DeepSeek API ({str(e)})"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: 没有提供用户意图")
        sys.exit(1)
        
    user_intent = sys.argv[1]
    system_prompt = f"""你是一个运行在 Ubuntu 操作系统底层的高级命令翻译引擎。
你的唯一任务是将用户的自然语言意图，精准翻译为极其专业的单行 Linux Shell 命令。

【严格规则】：
1. 你的输出必须只有一行原始命令，绝对不能包含任何问候、解释或多余的标点。
2. 绝对不能使用 Markdown 代码块符号（严禁输出 ```bash 和 ```）。
3. 请尽可能展现专业性，善用管道符 `|`、`grep`、`awk`、`xargs`、`find` 等高阶工具。
4. 如果用户的输入不是操作系统的操作意图（如问候、聊天），请固定输出：echo "未知的系统操作意图，请重新输入。"

【翻译示例】：
输入：帮我找出当前目录下占用空间最大的3个文件
输出：ls -alS | head -n 4

输入：杀掉所有名字里带有 nginx 的进程
输出：ps aux | grep nginx | grep -v grep | awk '{{print $2}}' | xargs kill -9

输入：{user_intent}
输出："""
    
    # 策略：先尝试本地大模型，如果失败再调用云端 API
    command = call_ollama(system_prompt)
    if not command:
        command = call_deepseek(user_intent)
        
    # 清理AI可能带有的多余反引号环境
    command = command.replace("```bash", "").replace("```", "").replace("`", "").strip()
    
    # 唯一输出，供 C 语言管道读取
    print(command)
