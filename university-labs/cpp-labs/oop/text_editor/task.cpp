#include "task.h"
#include "logged_command_wrapper.h"
#include <iostream>

class MoveCursorLeftCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        if(cursorPosition == 0)
            return;
        cursorPosition--;
        if(buffer[cursorPosition] == '\n')
        cursorPosition--;
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitMoveCursorLeftCommand(*this);
    }
    
};

class MoveCursorRightCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        if(cursorPosition == buffer.size())
            return;
        cursorPosition++;
        if(buffer[cursorPosition] == '\n')
            cursorPosition++;
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitMoveCursorRightCommand(*this);
    }
};

class MoveCursorUpCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        size_t PosEnterBefore = cursorPosition - 1;
        while(buffer[PosEnterBefore] != '\n' && PosEnterBefore != 0)
            PosEnterBefore--;
        if(PosEnterBefore == 0)
        {
            return;
        }
        size_t PosEnterBefore2 = PosEnterBefore - 1;
        while(PosEnterBefore2 != 0 && buffer[PosEnterBefore2] != '\n')
            PosEnterBefore2--;
        if(PosEnterBefore2 == 0)
        {
            PosEnterBefore2--;
        }
        size_t PosInLine = cursorPosition - PosEnterBefore;
        if(PosEnterBefore - PosEnterBefore2 - 1 <= PosInLine)
        {
            cursorPosition = PosEnterBefore;
            return;
        }
        cursorPosition = PosEnterBefore2 + PosInLine;
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitMoveCursorUpCommand(*this);
    }
};

class MoveCursorDownCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        size_t PosEnterBefore = cursorPosition - 1;
        if(cursorPosition == 0)
            PosEnterBefore = 0;
        while(buffer[PosEnterBefore] != '\n' && PosEnterBefore != 0)
            PosEnterBefore--;
        if(PosEnterBefore == 0)
            PosEnterBefore--;
        size_t PosInLine = cursorPosition - PosEnterBefore;
        size_t PosEnterAfter = cursorPosition;
        while(PosEnterAfter != buffer.size() && buffer[PosEnterAfter] != '\n')
            PosEnterAfter++;
        if(PosEnterAfter == buffer.size() || PosEnterAfter == buffer.size() - 1)
        {
            return;
        }
        size_t PosEnterAfter2 = PosEnterAfter + 1;
        while(PosEnterAfter2 != buffer.size() && buffer[PosEnterAfter2] != '\n')
            PosEnterAfter2++;
        if(PosEnterAfter2 - PosEnterAfter - 1 <= PosInLine)
        {
            return;
        }
        cursorPosition = PosEnterAfter + PosInLine;
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitMoveCursorDownCommand(*this);
    }
};


class SelectTextCommand : public ICommand {
    public:
    SelectTextCommand(size_t num):num_(num){}

    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        editor.SelectText(cursorPosition, num_ + cursorPosition);
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitSelectCommand(*this);
    }
    private:
    size_t num_;
};

class InsertTextCommand : public ICommand {
    public:
    InsertTextCommand(std::string & text):text_(text){}
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        if (editor.HasSelection())
        {
            auto p = editor.GetSelection();
            buffer.erase(p.first, p.second - p.first);
            cursorPosition = p.first;
            editor.UnselectText();

        }
        buffer.insert(cursorPosition, text_);
        cursorPosition += text_.size();
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitInsertTextCommand(*this);
    }
    private:
    std::string text_;
};

class DeleteTextCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override
    {
        if (editor.HasSelection())
        {
            auto p = editor.GetSelection();
            buffer.erase(p.first, p.second - p.first);
            cursorPosition = p.first;
            editor.UnselectText();
        }
        else
        {
            buffer.erase(cursorPosition, 1);
        }
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitDeleteTextCommand(*this);
    }
};

class CopyTextCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        if(editor.HasSelection())
        {
            clipboard.clear();
            auto p = editor.GetSelection();
            for(size_t pos = p.first; pos < p.second; pos++)
                clipboard+=buffer[pos];
        }
        else
        {
            clipboard.clear();
            clipboard += buffer[cursorPosition];
        }
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitCopyTextCommand(*this);
    }
};

class PasteTextCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        if (editor.HasSelection())
        {
            auto p = editor.GetSelection();
            buffer.erase(p.first, p.second - p.first);
            cursorPosition = p.first;
            editor.UnselectText();

        }
        buffer.insert(cursorPosition, clipboard);
        cursorPosition += clipboard.size();
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitPasteTextCommand(*this);
    }
};

class UppercaseTextCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        if(!editor.HasSelection())
            return;
        auto p = editor.GetSelection();
        for(size_t pos = p.first; pos < p.second; pos++)
        {
            if('a' <= buffer[pos] && buffer[pos] <= 'z'){
                buffer[pos] = char(int(buffer[pos]) - 32);
            }
        }
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitUppercaseTextCommand(*this);
    }
};

class LowercaseTextCommand : public ICommand {
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        if(!editor.HasSelection())
            return;
        auto p = editor.GetSelection();
        for(size_t pos = p.first; pos < p.second; pos++)
        {
            if('A' <= buffer[pos] && buffer[pos] <= 'Z')
                buffer[pos] = char(int(buffer[pos]) + 32);
        }
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitLowercaseTextCommand(*this);
    }
};

class MoveToEndCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        size_t PosEnterAfter = cursorPosition;
        while(PosEnterAfter < buffer.size() && buffer[PosEnterAfter] != '\n')
            PosEnterAfter++;
        cursorPosition = PosEnterAfter;
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitMoveToEndCommand(*this);
    }

};

class MoveToStartCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        size_t PosEnterBefore = cursorPosition - 1;
        while(PosEnterBefore != 0 && buffer[PosEnterBefore] != '\n')
            PosEnterBefore--;
        if(PosEnterBefore == 0)
        PosEnterBefore--;
            PosEnterBefore++;
        cursorPosition = PosEnterBefore;
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitMoveToStartCommand(*this);
    }
};

class DeleteWordCommand : public ICommand 
{
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        size_t PosEnterAfter = cursorPosition;
        while(PosEnterAfter < buffer.size() && buffer[PosEnterAfter] != '\n' && buffer[PosEnterAfter] != ' ')
            PosEnterAfter++;
        buffer.erase(cursorPosition, PosEnterAfter - cursorPosition);
        editor.UnselectText();
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        visitor.VisitDeleteWordCommand(*this);
    }
};

class MacroCommand : public ICommand {
    public:
    MacroCommand(std::list<CommandPtr> &l):l_(l){}
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor)
    {
        for(auto com: l_) 
        {
            com->Apply(buffer, cursorPosition, clipboard, editor);
        }
    }
    void AcceptVisitor(CommandVisitor & visitor) override
    {
        for(auto com: l_)
        {
            com ->AcceptVisitor(visitor);
        }
    }

    private:
    const std::list<CommandPtr> l_;
};

CommandBuilder::CommandBuilder()
{
    selectionSize_ = 0;
    logStreamPtr_ = nullptr;
}

CommandPtr CommandBuilder::build()
{
        CommandPtr p1;
        switch(type_)
        {
            case Type::MoveCursorLeft: p1 = CommandPtr(new MoveCursorLeftCommand()); break;
            case Type::MoveCursorRight: p1 = CommandPtr(new MoveCursorRightCommand()); break;
            case Type::MoveCursorUp: p1 = CommandPtr(new MoveCursorUpCommand()); break;
            case Type::MoveCursorDown: p1 = CommandPtr(new MoveCursorDownCommand()); break;
            case Type::SelectText: p1 = CommandPtr(new SelectTextCommand(selectionSize_)); break;
            case Type::InsertText: p1 = CommandPtr(new InsertTextCommand(text_)); break;
            case Type::DeleteText: p1 = CommandPtr(new DeleteTextCommand()); break;
            case Type::CopyText: p1 = CommandPtr(new CopyTextCommand()); break;
            case Type::PasteText: p1 = CommandPtr(new PasteTextCommand()); break;
            case Type::UppercaseText: p1 = CommandPtr(new UppercaseTextCommand()); break;
            case Type::LowercaseText: p1 = CommandPtr(new LowercaseTextCommand()); break;
            case Type::MoveToEnd: p1 = CommandPtr(new MoveToEndCommand()); break;
            case Type::MoveToStart: p1 = CommandPtr(new MoveToStartCommand()); break;
            case Type::DeleteWord: p1 = CommandPtr(new DeleteWordCommand()); break;
            case Type::Macro: p1 = CommandPtr(new MacroCommand(subcommands_)); break;
        }
        if(logStreamPtr_ != nullptr)
        {
            return CommandPtr(new LoggedCommandWrapper(*logStreamPtr_, p1));
        }
        return p1;
}

CommandBuilder& CommandBuilder::WithType(Type type)
{
    type_ = type;
    return *this;
}
CommandBuilder& CommandBuilder::SelectionSize(size_t selectionSize)
{
    selectionSize_ = selectionSize;
    return *this;
}
CommandBuilder& CommandBuilder::Text(std::string text)
{
    text_ = text;
    return *this;
}
CommandBuilder& CommandBuilder::LogTo(std::ostream& logStream)
{
    logStreamPtr_ = &logStream;
    return *this;
}
CommandBuilder& CommandBuilder::AddSubcommand(CommandPtr subcommand)
{
    subcommands_.push_back(subcommand);
    return *this;
}






void CommandLoggerVisitor::VisitMoveCursorLeftCommand(MoveCursorLeftCommand& command)
{
    logStream_ << 'h';   
}
void CommandLoggerVisitor::VisitMoveCursorRightCommand(MoveCursorRightCommand& command)
{
    logStream_ << 'l';   
}
void CommandLoggerVisitor::VisitMoveCursorUpCommand(MoveCursorUpCommand& command)
{
    logStream_ << 'k';   
}
void CommandLoggerVisitor::VisitMoveCursorDownCommand(MoveCursorDownCommand& command)
{
    logStream_ << 'j';   
}
void CommandLoggerVisitor::VisitSelectCommand(SelectTextCommand& command)
{
    logStream_ << 'v';   
}
void CommandLoggerVisitor::VisitInsertTextCommand(InsertTextCommand& command)
{
    logStream_ << 'i';   
}
void CommandLoggerVisitor::VisitDeleteTextCommand(DeleteTextCommand& command)
{
    logStream_ << 'd';   
}
void CommandLoggerVisitor::VisitCopyTextCommand(CopyTextCommand& command)
{
    logStream_ << 'y';   
}
void CommandLoggerVisitor::VisitPasteTextCommand(PasteTextCommand& command)
{
    logStream_ << 'p';   
}
void CommandLoggerVisitor::VisitUppercaseTextCommand(UppercaseTextCommand& command)
{
    logStream_ << 'U';   
}
void CommandLoggerVisitor::VisitLowercaseTextCommand(LowercaseTextCommand& command)
{
    logStream_ << 'u';   
}
void CommandLoggerVisitor::VisitMoveToEndCommand(MoveToEndCommand& command)
{
    logStream_ << '$';   
}
void CommandLoggerVisitor::VisitMoveToStartCommand(MoveToStartCommand& command)
{
    logStream_ << '0';   
}
void CommandLoggerVisitor::VisitDeleteWordCommand(DeleteWordCommand& command)
{
    logStream_ << "dE";   
}


CommandLoggerVisitor::CommandLoggerVisitor(std::ostream& ss)
    :logStream_(ss)
{}