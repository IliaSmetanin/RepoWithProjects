FROM ubuntu:24.04

# since ubuntu:24.04 pip cannot load packages globally
# necessary to create virtual environment
RUN apt update && apt install -y python3 python3-pip python3-venv libpq-dev curl

# venv - integrated module in python3 for creating virt env
RUN python3 -m venv /opt/venv 

COPY "requirements.txt" .
RUN /opt/venv/bin/pip install -r "requirements.txt"
# for outdated format of request
RUN /opt/venv/bin/pip uninstall -y fastapi starlette
# --force-reinstall - rewtrite existing packages
# --no-spes - turn the conflict dependencies check off
RUN /opt/venv/bin/pip install --force-reinstall --no-deps fastapi==0.108.0 starlette==0.33.0

WORKDIR /backend
COPY base.py init_db.py main.py ./
COPY templates/ ./templates
# :"$PATH" means adding to existed
ENV PATH="/opt/venv/bin/:$PATH" 
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]